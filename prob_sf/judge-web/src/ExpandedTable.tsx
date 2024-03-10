import {useFullscreen} from 'ahooks';
import {Modal, Table, message, Alert, Button} from 'antd';
import type {ColumnType} from 'antd/lib/table';
import type {FC} from 'react';
import React, {useEffect, useRef, useState} from 'react';
import Video from './icons/Video';
import MapReplays from './MapReplays';
import {getResult, getMapReplay, Response, IReplayResponse, runCode} from './api';
import {AxiosResponse} from 'axios';
import formatTimeUsed from './timeUsed';

interface IProps {
    /** 是否展开 */
    expanded: boolean;
    /** submission_id */
    submission_id?: string;
    fatherFull: boolean;
}

export const ITEM_WIDTH = 32;
export const ZONE_MIN_WIDTH = 724;
export const ZONE_MIN_HEIGHT = 451;
export const MAP_MIN_WIDTH = ZONE_MIN_WIDTH + 10;
export const MAP_MIN_HEIGHT = ZONE_MIN_HEIGHT + 110;

export const getZoneSize = (x: any, y: any) => {
    const width = (x + 1) * ITEM_WIDTH;
    const height = (y + 1) * ITEM_WIDTH;
    return {
        zoneWidth: width < ZONE_MIN_WIDTH ? ZONE_MIN_WIDTH : width,
        zoneHeight: height < ZONE_MIN_HEIGHT ? ZONE_MIN_HEIGHT : height
    };
};

const getReplaysAndMapAttr = (res: AxiosResponse<Response<IReplayResponse>>) => {
    let replays = res?.data?.data?.replays ?? [];
    let mapAttr = res?.data?.data?.map_attr ?? {width: 0, height: 0};

    if (Array.isArray(replays)) {
        if (!replays.length) {
            message.error('回放数据为空，请检查代码是否正确');
        }
    } else {
        // 如果跑出来的回放数据不是数组，给出提示
        replays = [];
        message.error('回放数据不为数组，请检查代码是否正确');
    }

    if (Object.prototype.toString.call(mapAttr) !== '[object Object]') {
        mapAttr = {width: 0, height: 0};
        message.error('解析地图信息失败，请检查地图信息的数据类型是否正确');
    } else if (!mapAttr.width || !mapAttr.height) {
        if (!mapAttr.width && !mapAttr.height) {
            message.error('检测到地图的宽度和高度不正确，请检查代码是否正确')
        } else if (!mapAttr.width) {
            message.error('检测到地图的宽度不正确，请检查代码是否正确');
        } else if (!mapAttr.height) {
            message.error('检测到地图的宽度不正确，请检查代码是否正确');
        }
        mapAttr = {width: 0, height: 0};
    }

    return {replays, mapAttr};
}

const ExpandedTable: FC<IProps> = props => {
    const ref = useRef(null);
    const [isFullscreen, {toggleFullscreen: toggleFull}] = useFullscreen(ref);

    const {expanded, submission_id = '0', fatherFull = false} = props;

    const [loading, setLoading] = useState(false);
    const [dataSource, setDataSource] = useState<any[] | []>([]);
    const [replays, setReplays] = useState<any>([]);
    const [mapAttr, setMapAttr] = useState<any>({width: 0, height: 0});
    const [isFullscreenRadio, setIsFullscreenRadio] = useState(1);
    const [isRunning, setIsRunning] = useState(false);

    const getReplays = ({mapId}: {mapId: string}) => {
        getMapReplay(mapId)
            .then((res) => {
                const {replays, mapAttr} = getReplaysAndMapAttr(res);
                setReplays(replays);
                setMapAttr(mapAttr);
            })
            .catch(() => {
                setReplays([]);
                setMapAttr({width: 0, height: 0});
            });
    };

    const getData = () => {
        setLoading(true);
        getResult().then(res => {
            const list = res.data.data.extras.testcases.map((item, index) => ({
                key: `${Date.now() + index}`,
                map_id: item.map_id,
                steps: item.steps,
                success: item.done,
                invalid_actions: item.invalid_actions,
                status: item.status,
                time_used: item.time_used
            }))
            setDataSource(list);
        }).finally(() => {
            setLoading(false);
        })
    };

    useEffect(() => {
        getData();
        const interval = setInterval(() => {
            if (isRunning) return;
            getResult().then(res => {
                const list = res.data.data.extras.testcases.map((item, index) => ({
                    key: `${Date.now() + index}`,
                    map_id: item.map_id,
                    steps: item.steps,
                    success: item.done,
                    invalid_actions: item.invalid_actions,
                    status: item.status,
                    time_used: item.time_used
                }))
                setDataSource(list);
            })
        }, 5000)
        return () => {
            clearInterval(interval)
        }
    }, [isRunning, expanded, submission_id]);

    const columns: ColumnType<any>[] = [
        {
            title: '地图',
            key: 'map_id',
            dataIndex: 'map_id'
        },
        {
            title: '当局步数',
            key: 'steps',
            dataIndex: 'steps'
        },
        {
            title: '无效指令数',
            key: 'invalid_actions',
            dataIndex: 'invalid_actions'
        },
        {
            title: '是否完成',
            key: 'success',
            dataIndex: 'success',
            render: text => (text ? '是' : '否')
        },
        {
            title: '状态',
            key: 'status',
            dataIndex: 'status',
            render: status => status
        },
        {
            title: '耗时(秒)',
            key: 'time_used',
            dataIndex: 'time_used',
            width: 80,
            render: text => <span style={{whiteSpace: 'pre'}}>{text.toFixed(3)}</span>
        },
        {
            title: '操作',
            key: 'operation',
            dataIndex: 'operation',
            render: (_, {map_id}) => (
                <div
                    style={{fontSize: 12, color: '#1677ff', cursor: isRunning ? 'not-allowed' : 'pointer'}}
                    onClick={() => getReplays({mapId: map_id} as {mapId: string})
                }>查看回放</div>
            )
        }
    ];

    const switchFullScreen = () => {
        toggleFull();
    };

    const onRunCode = () => {
        runCode();
        setIsRunning(true);
        message.info('运行中...')
        const _runningInterval = window.setInterval(() => {
            runCode().then(res => {
                const result = res.data;
                if (result.status === 'finish') {
                    if (result.code === 0) {
                        message.success('运行成功!')
                    } else {
                        message.error(JSON.stringify(result?.error ?? ''))
                    }
                    setIsRunning(false);
                    window.clearInterval(_runningInterval)
                } else if (result.status === 'running') {
                    // do something
                }
            })
        }, 5000)
    }

    useEffect(() => {
        const zoneSize = getZoneSize(mapAttr.width, mapAttr.height);
        if (!isFullscreen) {
            setIsFullscreenRadio(1);
        } else if (ref?.current) {
            const timer = window.setTimeout(() => {
                setIsFullscreenRadio(
                    Math.floor(
                        Math.min(
                            (ref.current as any).offsetHeight / (zoneSize.zoneHeight + 110),
                            (ref.current as any).offsetWidth / (zoneSize.zoneWidth + 10)
                        ) * 100
                    ) / 100
                );
                window.clearTimeout(timer);
            });
        }
    }, [isFullscreen, replays, mapAttr]);

    const zoneSize = getZoneSize(mapAttr.width, mapAttr.height);
    return (
        <>
            {/* <Alert
                style={{borderRadius: '0', border: 'none', width: '100%', height: '70px', fontSize: '16px', paddingLeft: '35px'}}
                message={
                    <>
                        <div>1. 地图的运行结果每经过 5s 刷新一次，数据源为你在本地执行完 bash run.sh 后生成的 node-judger/replays/ 目录下的文件</div>
                        <div>2. 点击运行后，将会自动执行 bash run.sh，运行期间，下列表格数据将暂停刷新。运行结束后，表格数据将会更新，并且恢复每隔 5s 刷新一次</div>
                    </>
                }
                type="info"
                action={<Button type="primary" onClick={onRunCode} style={{ marginRight: '20px' }} disabled={isRunning}>运行</Button>}
            /> */}
            <Alert
                style={{borderRadius: '0', border: 'none', width: '100%', height: '70px', fontSize: '16px', paddingLeft: '35px'}}
                message={
                    <strong>
                        <span style={{color: 'red'}}>注意：</span>
                        地图的运行结果每经过 5s 刷新一次，数据源为你在本地执行完
                        <span style={{color: 'red'}}> bash run.sh </span>
                        后生成的
                        <span style={{color: 'red'}}> node-judger/replays/ </span>
                        目录下的文件
                    </strong>
                }
                type="info"
            />
            <Table
                loading={loading}
                dataSource={dataSource}
                columns={columns}
                key="game_id"
                size="small"
                pagination={false}
                scroll={{y: 1000}}
                style={{margin: '20px'}}
            />
            <Modal
                footer={null}
                onCancel={() => {
                    setReplays([]);
                    setMapAttr({width: 0, height: 0});
                }}
                open={replays?.length > 0}
                title={
                    <span style={{display: 'flex', justifyContent: 'flex-start', alignItems: 'center'}}>
                        <Video />
                        <span style={{marginLeft: 16}}>地图回放</span>
                    </span>
                }
                style={{top: 10, minWidth: 800, overflow: 'auto'}}
                centered
                destroyOnClose
                getContainer={() => document.getElementById('algo-left') || document.body}>
                <div
                    ref={ref}
                    style={{
                        width: isFullscreen ? '100vw' : 'auto',
                        height: isFullscreen ? '100vh' : 'auto'
                    }}>
                    <MapReplays
                        isFullscreenRadio={isFullscreenRadio}
                        replays={replays}
                        switchFullscreen={switchFullScreen}
                        fatherFull={fatherFull}
                        zoneHeight={zoneSize.zoneHeight}
                        zoneWidth={zoneSize.zoneWidth}
                    />
                </div>
            </Modal>
        </>
    );
};

export default ExpandedTable;
