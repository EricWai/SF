
import requests
from uuid import uuid4
import time
import os
SERVER_URL  = os.environ.get("sf-judge-server") or "http://127.0.0.1:5555"
print(SERVER_URL)
USERNAME = "算法挑战赛-AI平台"
SUBMISSION_ID = str(uuid4())
ACTIONS_SEQ = [
    [{"type":"PICKUP","dir":"RIGHT"},{"type":"PICKUP","dir":"LEFT"}],
    [{"type":"MOVE","dir":"RIGHT"},{"type":"MOVE","dir":"LEFT"}],
    [{"type":"MOVE","dir":"LEFT"},{"type":"MOVE","dir":"LEFT"}],
    [{"type":"MOVE","dir":"RIGHT"},{"type":"STAY"}],
    [{"type":"MOVE","dir":"UP"},{"type":"STAY"}],
    [{"type":"STAY"},{"type":"MOVE","dir":"LEFT"}],
    [{"type":"STAY"},{"type":"MOVE","dir":"LEFT"}],
    [{"type":"MOVE","dir":"DOWN"},{"type":"MOVE","dir":"LEFT"}],
    [{"type":"MOVE","dir":"RIGHT"},{"type":"MOVE","dir":"LEFT"}],
    [{"type":"MOVE","dir":"RIGHT"},{"type":"DELIVERY","dir":"LEFT"}],
    [{"type":"MOVE","dir":"RIGHT"},{"type":"STAY"}],
    [{"type":"DELIVERY","dir":"RIGHT"},{"type":"STAY"}]
]


if __name__ == "__main__":

    resp = requests.post(f"{SERVER_URL}/create_submission")
    assert (resp.status_code == 200)
    data = resp.json()
    print ("create_submission: ", data)

    for map_id in data['value']['maps']:
        print(map_id)
        resp = requests.post(f"{SERVER_URL}/start",json={"map_id":map_id})
        assert (resp.status_code == 200)
        data = resp.json()
        print (data)
        for actions in ACTIONS_SEQ:
            try:
                print('actions:', actions)
                resp = requests.post(f"{SERVER_URL}/step",json={"map_id": map_id,"actions": actions})
                assert (resp.status_code == 200)
                data = resp.json()
                if data["value"]["done"]:
                    print ('done:', data["value"]["done"])
                    break
            except AssertionError:
                print("assert error occur")
                break;

    resp = requests.post(f"{SERVER_URL}/finish_submission",json={})
    assert (resp.status_code == 200)
    data = resp.json()
    print (data)





