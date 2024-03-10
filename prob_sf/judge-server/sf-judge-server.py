from cmath import log
import os
import sys
import time
from schema import Schema, And, Use, Optional, SchemaError
import random
import copy
import json
from functools import wraps
import asyncio

from sanic import Sanic, BadRequest, Forbidden, response, NotFound
from dataclasses import dataclass
import logging
import threading


logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
# 配置控制台处理器
console_handler = logging.StreamHandler()
console_handler.setLevel(logging.INFO)
console_formatter = logging.Formatter('%(asctime)s - %(process)d - %(thread)d - %(name)s - %(levelname)s - %(message)s')
console_handler.setFormatter(console_formatter)
logger.addHandler(console_handler)

@dataclass
class AGV:
    id: int
    payload: int
    cap: int
    x: int
    y: int


@dataclass
class Shelf:
    id: int
    payload: int
    cap: int


@dataclass
class Cargo:
    id: int
    weight: int
    target: int


@dataclass
class Cell:
    _type: str
    id: int


class Timeout(Exception): pass

class InvalidAction(Exception): pass

class NotFound(Exception): pass

#INIT -> RUNNING -> SUCCESS
#
class Env:
    """docstring for Env"""

    def __init__(self, map_id, init_map):
        super(Env, self).__init__()
        self.map_id = map_id

        self.init_map = init_map
        print(map_id, init_map['map_attr'])
        if init_map != None:
            self.map_attr = init_map['map_attr']

        self._map = {}
        self.agvs = []
        self.cargos = []
        self.shelves = []

        self.done = False
        self.create_at = time.time()
        self.last_time = time.time()
        self.steps = 0
        self.invalid_actions = 0
        self.replays = []
        self.prev_state = None
        self.status = 'INIT'


    def to_json(self):
        jsonMap = self.get_state()

        jsonMap['map_id'] = self.map_id
        jsonMap['init_map'] = self.init_map
        jsonMap['map_attr'] = self.init_map['map_attr']
        jsonMap['last_time'] = self.last_time
        jsonMap['steps'] = self.steps
        jsonMap['invalid_actions'] = self.invalid_actions
        jsonMap['prev_state'] = self.prev_state
        jsonMap['create_at'] = self.create_at
        return jsonMap

    def from_obj(self, jsonMap):
        # jsonMap = self.get_state()
        self.agvs = [AGV(a['id'], a['payload'], a['cap'], a['x'], a['y']) for a in jsonMap['agvs']]
        self.cargos = [Cargo(a['id'], a['weight'], a['target']) for a in jsonMap['cargos']]
        self.shelves = [Shelf(a['id'], a['payload'], a['cap']) for a in jsonMap['shelves']]

        for v in jsonMap['map']:
            self._map[v['x'], v['y']] = Cell(v['type'], v['id'])

        self.map_id = jsonMap['map_id']
        self.init_map = jsonMap['init_map']
        self.map_attr = jsonMap['map_attr']
        self.last_time = jsonMap['last_time']
        self.steps = jsonMap['steps']
        self.invalid_actions = jsonMap['invalid_actions']
        self.prev_state = jsonMap['prev_state']
        self.create_at = jsonMap['create_at']
        self.done = False
        return self

    def reset(self):
        init_state = self.init_map['map_state']

        agvs = sorted(init_state['agvs'], key=lambda x: x['id'])
        for _id, agv in enumerate(agvs):
            assert (_id == agv['id'])
            self.agvs.append(AGV(_id, agv.get('payload'), agv.get('cap', 1), None, None))

        cargos = sorted(init_state['cargos'], key=lambda x: x['id'])
        for _id, cargo in enumerate(cargos):
            assert (_id == cargo['id'])
            self.cargos.append(Cargo(_id, cargo.get('weight', 1), cargo['target']))

        shelves = sorted(init_state['shelves'], key=lambda x: x['id'])
        for _id, shelf in enumerate(shelves):
            assert (_id == shelf['id'])
            self.shelves.append(Shelf(_id, shelf.get('payload'), shelf.get('cap', 1)))

        assert (len(shelves) == len(cargos))

        for cell in init_state['maps']:
            x, y = cell['x'], cell['y']
            id = cell['id']
            if id is not None:
                id = int(id)

            _type = cell['type']
            self._map[x, y] = Cell(_type, id)

            if cell['type'] == 'agv':
                self.agvs[id].x = x
                self.agvs[id].y = y

        self.create_at = time.time()
        self.last_time = time.time()
        self.steps = 0
        state = self.get_state()
        self.prev_state = state
        self.status = 'RUNNING'

        return self.map_attr.copy(), state

    # def reset(self):
    # 	init_state = self.init_map['map_state']

    # 	for _id,agv in enumerate(init_state['agvs']):
    # 		x,y = agv['x'],agv['y']
    # 		self.agvs.append(AGV(_id,agv.get('payload'),agv.get('cap',1),x,y))
    # 		self._map[x,y] = Cell('agv',_id)

    # 	for _id,cargo in enumerate(init_state['cargos']):
    # 		x,y = cargo['x'],cargo['y']
    # 		self.cargos.append(Cargo(_id,cargo.get('weight',1),cargo['target']))
    # 		self._map[x,y] = Cell('cargo',_id)

    # 	for _id,wall in enumerate(init_state['walls']):
    # 		x,y = wall['x'],wall['y']
    # 		self._map[x,y] = Cell('wall',None)

    # 	for _id,shelf in enumerate(init_state['shelves']):
    # 		x,y = shelf['x'],shelf['y']
    # 		self.shelves.append(Shelf(_id,shelf.get('payload'),shelf.get('cap',1)))
    # 		self._map[x,y] = Cell('shelf',_id)

    # 	self.last_time = time.time()
    # 	self.create_at = time.time()
    # 	self.steps = 0
    # 	state = self.get_state()
    # 	self.prev_state = state

    # 	return self.map_attr.copy(),state

    def get_state(self):
        agvs = [{"id": a.id, "payload": a.payload, "cap": a.cap, "x": a.x, "y": a.y} for a in self.agvs]
        cargos = [{"id": c.id, "weight": c.weight, "target": c.target} for c in self.cargos]
        shelves = [{"id": s.id, "payload": s.payload, "cap": s.cap} for s in self.shelves]
        _map = [{"x": x, "y": y, "type": cell._type, "id": cell.id} for (x, y), cell in self._map.items()]

        state = {
            "agvs": agvs,
            "cargos": cargos,
            "shelves": shelves,
            "map": _map,
        }

        return state

    def _pickup(self, agv, pos, prev_poses):
        x, y = pos

        # agv_x,agv_y = agv.x,agv.y
        # if (x,y) not in ((agv_x+1,agv_y),(agv_x-1,agv_y),(agv_x,agv_y+1),(agv_x,agv_y-1)):
        # 	self.invalid_actions += 1
        # 	logger.debug('invalid pickup action, not in 4 directions')
        # 	return

        cell = self._map.get((x, y), None)
        logger.debug("agv: %s,action: %s, cell: %s", agv, pos, cell)
        if cell is None:
            self.invalid_actions += 1
            logger.debug('invalid pickup action, nothing here')
            return

        # agv已经有货
        if agv.payload is not None:
            self.invalid_actions += 1
            logger.debug('invalid pickup action, agv is full')
            return

        # 从地上拿货
        if cell._type == 'cargo':
            # 更新agv和地图状态
            agv.payload = cell.id
            self._map.pop((x, y))

        # 从货架上拿
        elif cell._type == 'shelf':
            shelf = self.shelves[cell.id]

            if shelf.payload is None:
                self.invalid_actions += 1
                logger.debug('invalid pickup action, shelf is empty')
                return

            # 更新shelf和agv状态
            agv.payload = shelf.payload  ##cell.id
            shelf.payload = None

        # 其他情形
        else:
            logger.debug('invalid pickup action, not a cargo')
            self.invalid_actions += 1

    def _delivery(self, agv, pos, prev_poses):
        x, y = pos

        # agv_x,agv_y = agv.x,agv.y
        # if (x,y) not in ((agv_x+1,agv_y),(agv_x-1,agv_y),(agv_x,agv_y+1),(agv_x,agv_y-1)):
        # 	self.invalid_actions += 1
        # 	logger.debug('invalid delivery action, not in 4 directions')
        # 	return

        if x >= self.map_attr['width'] or x < 0:
            logger.debug('invalid delivery action, out of map')
            self.invalid_actions += 1
            return

        if y >= self.map_attr['height'] or y < 0:
            logger.debug('invalid delivery action, out of map')
            self.invalid_actions += 1
            return

        # agv为空
        if agv.payload is None:
            self.invalid_actions += 1
            logger.debug('invalid pickup action, agv is empty')
            return

        cell = self._map.get((x, y), None)
        logger.debug("agv: %s,action: %s, cell: %s", agv, pos, cell)

        # 放在路上
        if cell is None:

            # 之前该位置不为空
            if (x, y) in prev_poses:
                logger.debug('invalid delivery action, full shelf')
                self.invalid_actions += 1
                return

            # 更新地图和agv的payload
            self._map[x, y] = Cell('cargo', agv.payload)
            agv.payload = None

        # 放在货架里面
        elif cell._type == 'shelf':
            shelf = self.shelves[cell.id]

            # 货架不为空
            if shelf.payload is not None:
                logger.debug('invalid delivery action, full shelf')
                self.invalid_actions += 1
                return

            # 更新货架和agv的payload
            shelf.payload = agv.payload
            agv.payload = None

        # 其他情形
        else:
            logger.debug('invalid delivery action, wrong cell type')
            self.invalid_actions += 1

    def _move(self, agv, pos, prev_poses):
        x, y = pos
        width, height = self.map_attr['width'], self.map_attr['height']

        if x >= width or x < 0:
            logger.debug('invalid move action, out of map')
            self.invalid_actions += 1
            return

        if y >= height or y < 0:
            logger.debug('invalid move action, out of map')
            self.invalid_actions += 1
            return

        # agv要去的地方已经有物体 或者 前一个时刻有物体
        if (x, y) in self._map or (x, y) in prev_poses:
            logger.debug('invalid move action, collision!!')
            self.invalid_actions += 1
            return

        # update 地图信息
        cell = self._map.pop((agv.x, agv.y))
        self._map[x, y] = cell
        # update agv 坐标
        agv.x = x
        agv.y = y

    def _step(self, agv, action, prev_poses):
        _type = action['type']
        _dir = action.get('dir')
        x, y = agv.x, agv.y

        #print(x, y)
        if _dir == "UP":
            target_pos = (x, y - 1)
        elif _dir == "DOWN":
            target_pos = (x, y + 1)
        elif _dir == "LEFT":
            target_pos = (x - 1, y)
        elif _dir == "RIGHT":
            target_pos = (x + 1, y)
        else:
            if _type != "STAY":
                self.invalid_actions += 1
                logger.debug("invalid action: %s", action)
                raise InvalidAction(f"invalid action : {action} ")

        if _type == "STAY":
            pass
        elif _type == "PICKUP":
            self._pickup(agv, target_pos, prev_poses)
        elif _type == "DELIVERY":
            self._delivery(agv, target_pos, prev_poses)
        elif _type == "MOVE":
            self._move(agv, target_pos, prev_poses)
        else:
            self.invalid_actions += 1
            logger.debug("invalid action: %s", action)
            raise InvalidAction(f"invalid action : {action} ")

    def is_done(self):
        for cargo in self.cargos:
            if self.shelves[cargo.target].payload != cargo.id:
                return False
        return True

    def step(self, actions):
        now = time.time()
        duration = now - self.last_time

        self.steps += 1
        self.last_time = now

        if duration > self.map_attr['timeout']:
            raise Timeout(f"time out: {duration}")

        if self.steps > self.map_attr['max_steps']:
            raise Timeout(f"max steps reached: {self.steps}")

        logger.debug("actions: %s", actions)
        self.replays.append({"map_state": self.prev_state, "actions": actions})

        prev_poses = set(self._map.keys())

        for agv, act in zip(self.agvs, actions):
            self._step(agv, act, prev_poses)

        self.done = self.is_done()
        state = self.get_state()
        self.prev_state = state

        if self.done:
            self.replays.append({"map_state": state, "actions": None})
            self.status = 'SUCCESS'

        return state, self.invalid_actions, self.steps, now - self.create_at, self.done


def json_api(schema=None):
    if schema:
        schema = Schema(schema)

    def _(f):
        @wraps(f)
        async def decorated_function(request, *args, **kwargs):
            try:
                if schema:
                    schema.validate(request.json)
                data, code = await f(request, *args, **kwargs)
                # logger.debug("data:%s" % data)
                return response.json({"msg": "ok", "value": data, "success": True}, status=200)

            except (BadRequest, SchemaError) as e:
                logger.error(e, exc_info=True)
                return response.json({"msg": str(e), "success": False}, status=400)
            except Forbidden as e:
                logger.error(e, exc_info=True)
                return response.json({"msg": str(e), "success": False}, status=403)
            except NotFound as e:
                logger.error(e, exc_info=True)
                return response.json({"msg": str(e), "success": False}, status=404)
            except Timeout as e:
                logger.error(e, exc_info=True)
                return response.json({"msg": str(e), "success": False, "reason": "timeout"}, status=400)
            except InvalidAction as e:
                logger.error(e, exc_info=True)
                return response.json({"msg": str(e), "success": False, "reason": "invalid action"}, status=400)
            except Exception as e:
                logger.error(e, exc_info=True)
                return response.json({"msg": str(e), "success": False}, status=500)

        return decorated_function

    return _


global maps, envs, replayFiles

maps = [f.split('.')[0] for f in os.listdir("./data")]
envs = {map_id: Env(map_id, json.load(open(f"./data/{map_id}.json"))) for map_id in maps}
replayFiles = { map_id: open(f"./replays/{map_id}-replay.json", mode="a+") for map_id in maps}

app = Sanic("sf-server")


@app.route("/create_submission", methods=["POST"])
@json_api({})
async def create_submission(request):
    data = request.json
    maps_copy = maps.copy()
    random.shuffle(maps_copy)
    return {"maps": maps_copy}, 200


@app.route("/start", methods=["POST"])
@json_api({
    'map_id': str,
})
async def start_game(request):
    data = request.json
    map_id = data['map_id']
    env = envs[map_id]
    map_attr, map_state = env.reset()
    return {"map_attr": map_attr, "map_state": map_state}, 200



@app.route("/step", methods=["POST"])
@json_api({
    'map_id': str,
    'actions': list,
})
async def step_game(request):
    actions = request.json['actions']
    map_id = request.json['map_id']
    env = envs[map_id]
    if env.status != 'RUNNING':
        raise NotFound(map_id)
    try:
        state, invalid_actions, steps, time_used, done = env.step(actions)
        print(f"write-replays: {map_id}, len: {len(env.replays)}")
        for replay in env.replays:
            replayFiles[map_id].write(json.dumps(replay)+"\n")
        env.replays = []
    except Timeout as e:
        env.status = 'TIMEOUT'
        raise e
    except InvalidAction as e:
        env.status = 'InvalidAction'
        raise e
    return {"map_state": state, "done": done, "steps": steps, "time_used": time_used,
            "invalid_actions": invalid_actions}, 200

async def exit_program():
    for map_id in maps:
        replays_obj = {}
        replays_obj["map_attr"] = {}
        replays_obj["replays"] = []

        map_data = envs[map_id].to_json()
        replays_obj["map_attr"] = map_data["map_attr"]

        with open(f"./replays/{map_id}-replay.json", "r") as f:
            for line in f:
                replay = json.loads(line)
                replays_obj['replays'].append(replay)
        # print(f"map: {map_id} replays: {replays}")
        with open(f"./replays/{map_id}-replay.json", "w") as f:
            json.dump(replays_obj, f,  ensure_ascii=False)
    print("exit succ")
    app.stop()

async def start_timer():
    await asyncio.sleep(3.0)

    asyncio.create_task(exit_program())


@app.route("/finish_submission", methods=["POST"])
@json_api({})
async def finish_submission(request):
    data = request.json
    success_count = 0
    steps = 0
    time_used = 0.0
    invalid_actions = 0

    s = set()
    testcases = []

    for map_id, env in envs.items():
        testcases.append({
            "create_at": env.create_at,
            "invalid_actions": env.invalid_actions,
            "last_time": env.last_time,
            "map_id": map_id,
            "steps": env.steps,
            "done": env.done,
            "status": env.status,
            "time_used": round(env.last_time, 3) - round(env.create_at, 3)
        })
        if env.done == True:
            success_count += 1
            steps += env.steps
            time_used += round(env.last_time, 3) - round(env.create_at, 3)
            invalid_actions += env.invalid_actions
        print(f"{map_id}, begin-close")
        replayFiles[map_id].close()
        print(f"{map_id}, end-close")

    final_result = {
        "code": 0,
        "score": 0.0,
        "extras":{
            "success_count": success_count,
            "steps": steps,
            "time_used": time_used,
            "invalid_actions": invalid_actions,
            "finish_at": time.time(),
            "testcases": testcases
        }
    }
    with open("./judgeResult.json", "w") as f:
        json.dump(final_result, f, indent=4, ensure_ascii=False)

    print("judge_result:", json.dumps(final_result, ensure_ascii=False))
    loop = asyncio.get_event_loop()
    loop.create_task(start_timer())
    logger.info('Timer started')

    return {"success_count": success_count, "steps": steps, "time_used": time_used
               , "invalid_actions": invalid_actions}, 200


if __name__ == '__main__':
    os.system("pwd");
    app.run(host="0.0.0.0", port=5555, workers=1)
