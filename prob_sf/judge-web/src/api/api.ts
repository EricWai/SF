import instance from './base';

export interface Response<T> {
    code: number;
    msg: string;
    data: T;
}

export interface Testcase {
    status: string;
    done: boolean;
    invalid_actions: number;
    last_time: number;
    map_id: string;
    steps: number;
    time_used: number;
}

export interface Replay {
    done: boolean;
    invalid_actions: number;
    last_time: number;
    map_id: string;
    steps: number;
}

export interface MapAttr {
    width: number;
    height: number;
    max_steps?: number;
    timeout?: number;
}

export interface IReplayResponse {
    replays: Replay[];
    map_attr: MapAttr;
}

export interface IJudgeResultResponse {
    extras: {
        testcases: Testcase[];
    };
    score: number;
}

/** 获取判题结果 */
export const getResult = () => instance.get<Response<IJudgeResultResponse>>('/result')
/** 获取地图回放数据 */
export const getMapReplay = (map: string) => instance.get<Response<IReplayResponse>>('/replay', {
    params: {map, test: 123}
})
/** 执行 bash run.sh */
export const runCode = () => instance.get<Response<any>>('/run')