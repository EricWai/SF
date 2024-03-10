import {Button, Space} from 'antd';
import classnames from 'classnames';
import Slider from 'rc-slider';
import 'rc-slider/assets/index.css';
import React from 'react';
import {ITEM_WIDTH} from './ExpandedTable';
import {handleRender} from './HandleTooltip';
import AgvLoaded from './icons/AgvLoaded';
import AgvUnloaded from './icons/AgvUnloaded';
import Cargo from './icons/Cargo';
import PauseSmall from './icons/PauseSmall';
import PlaySmall from './icons/PlaySmall';
import Shelf from './icons/Shelf';
import VideoFullscreen from './icons/VideoFullscreen';
import Wall from './icons/Wall';
import styles from './index.module.less';

// 默认帧率
const FPS = 1;

// Icon Map
const IconsMaps = {
    wall: <Wall />,
    cargo: <Cargo />,
    shelf: <Shelf />,
    agv: <AgvUnloaded />,
    agvpayload: <AgvLoaded />
};

interface IProps {
    switchFullscreen: () => void;
    replays: any[];
    isFullscreenRadio: number;
    fatherFull?: boolean;
    zoneWidth?: number;
    zoneHeight?: number;
}

class MapReplays extends React.Component<IProps, any> {
    countdownId?: number;
    sliderRef = React.createRef();

    constructor(props: IProps) {
        super(props);
        this.state = {
            currentFrame: 0,
            pause: true,
            fps: FPS
        };
    }

    handleSliderChange = (value: number | number[]) => {
        this.setState({currentFrame: value});
    };

    startTimer = () => {
        if (this.countdownId) return;

        this.countdownId = window.setInterval(() => {
            const {currentFrame, pause} = this.state;
            if (currentFrame + 1 < this.props.replays.length && !pause) {
                this.setState({currentFrame: currentFrame + 1});
            } else {
                this.stopTimer();
            }
        }, 1000 / this.state.fps);
    };

    stopTimer = () => {
        if (this.countdownId) {
            clearInterval(this.countdownId);
            this.countdownId = undefined;
        }
    };

    componentDidMount = () => {
        this.startTimer();
    };

    componentWillUnmount = () => {
        this.stopTimer();
        this.setState({pause: true, currentFrame: 0, fps: FPS});
    };

    handleMenuClick = ({key}: {key: string | number}) => {
        this.stopTimer();
        this.setState({fps: Number(key)}, () => {
            this.startTimer();
        });
    };

    render() {
        const { currentFrame, pause } = this.state;
        const oneFrame = this.props.replays[currentFrame];
        const { actions } = oneFrame;
        const map_state = oneFrame.map_state ?? { agvs: [], shelves: [], cargos: [], maps: [], map: [] };
        const { agvs, shelves, maps: _maps, map: _map } = map_state;
        const maps = _maps || _map;

        const getItem = (value: {type: 'wall' | 'cargo' | 'shelf' | 'agv' | 'agvpayload'; id: number}) => {
            if (value.type === 'agv') {
                const payload = agvs?.find((a: any) => a.id === value.id)?.payload;

                return (
                    <div className={styles.agvItem}>
                        {actions?.[value.id]?.dir ? (
                            <div className={styles.actionOne}>{actions[value.id].dir}</div>
                        ) : null}
                        <div className={styles.agvNumber}>{value.id}</div>
                        <div style={{transform: 'scale(1.5)'}}>
                            {payload !== undefined && payload !== null ? IconsMaps.agvpayload : IconsMaps.agv}
                        </div>
                        {actions?.[value.id]?.type ? (
                            <div className={styles.actionTwo}>{actions[value.id].type}</div>
                        ) : null}
                    </div>
                );
            } else if (value.type === 'shelf') {
                const payload = shelves?.find((a: any) => a.id === value.id)?.payload;
                return (
                    <div className={styles.agvItem}>
                        {payload !== undefined && payload !== null ? (
                            <div className={styles.cargoItem}>
                                <div className={styles.cargoNumber}>{payload}</div>
                                {IconsMaps.cargo}
                            </div>
                        ) : null}
                        <div style={{transform: 'scale(1.25)'}}>{IconsMaps.shelf}</div>
                    </div>
                );
            } else if (value.type === 'cargo') {
                return (
                    <div className={styles.cargoItem}>
                        <div className={styles.cargoNumber}>{value.id}</div>
                        {IconsMaps.cargo}
                    </div>
                );
            }
            return IconsMaps[value.type];
        };
        return (
            <div
                className={styles.map}
                style={{
                    transformOrigin: 'center top',
                    transform: 'scale(' + (this.props.isFullscreenRadio ?? 1) + ')',
                    height: this.props.zoneHeight ? this.props.zoneHeight + 110 : undefined,
                    width: this.props.zoneWidth ? this.props.zoneWidth + 10 : undefined
                }}>
                <div className={styles.toolbar}>
                    <div className={styles.tool}>
                        <Cargo />
                        <span>货物</span>
                    </div>
                    <div className={styles.tool}>
                        <Wall />
                        <span>障碍</span>
                    </div>
                    <div className={styles.tool}>
                        <Shelf />
                        <span>货架</span>
                    </div>
                    <div className={styles.tool}>
                        <AgvUnloaded />
                        <span>空车</span>
                    </div>
                    <div className={styles.tool}>
                        <AgvLoaded />
                        <span>载车</span>
                    </div>
                </div>
                <div
                    className={styles.zone}
                    style={{
                        width: this.props.zoneWidth ? this.props.zoneWidth : undefined,
                        height: this.props.zoneHeight ? this.props.zoneHeight : undefined
                    }}>
                    <div className={styles.bg}>
                        <div className={styles.box}>
                            {maps
                                .filter((m: any) => m.type === 'wall')
                                .sort((a: any, b: any) => a.x + a.y - (b.x + b.y))
                                .map((m: any, index: any) => ({...m, index}))
                                .map((value: any) => (
                                    <div
                                        className={styles.item}
                                        key={value.index}
                                        style={{
                                            transform:
                                                'translate(' +
                                                value.x * ITEM_WIDTH +
                                                'px,' +
                                                value.y * ITEM_WIDTH +
                                                'px)'
                                        }}>
                                        {getItem(value)}
                                    </div>
                                ))}
                            {maps
                                .filter((m: any) => m.type === 'agv')
                                .sort((a: any, b: any) => a.id - b.id)
                                .map((value: any) => (
                                    <div
                                        className={styles.item}
                                        key={Number(value.id) + value.type}
                                        style={{
                                            transform:
                                                'translate(' +
                                                value.x * ITEM_WIDTH +
                                                'px,' +
                                                value.y * ITEM_WIDTH +
                                                'px)'
                                        }}>
                                        {getItem(value)}
                                    </div>
                                ))}
                            {maps
                                .filter((m: any) => m.type === 'cargo')
                                .sort((a: any, b: any) => a.id - b.id)
                                .map((value: any) => (
                                    <div
                                        className={styles.item}
                                        key={Number(value.id) + value.type}
                                        style={{
                                            transform:
                                                'translate(' +
                                                value.x * ITEM_WIDTH +
                                                'px,' +
                                                value.y * ITEM_WIDTH +
                                                'px)'
                                        }}>
                                        {getItem(value)}
                                    </div>
                                ))}
                            {maps
                                .filter((m: any) => m.type === 'shelf')
                                .sort((a: any, b: any) => a.id - b.id)
                                .map((value: any) => (
                                    <div
                                        className={styles.item}
                                        key={Number(value.id) + value.type}
                                        style={{
                                            transform:
                                                'translate(' +
                                                value.x * ITEM_WIDTH +
                                                'px,' +
                                                value.y * ITEM_WIDTH +
                                                'px)'
                                        }}>
                                        {getItem(value)}
                                    </div>
                                ))}
                        </div>
                    </div>
                </div>
                <div className={styles.controller}>
                    <div className={styles.slider}>
                        <Slider
                            ref={this.sliderRef as any}
                            min={0}
                            step={1}
                            max={this.props.replays.length - 1}
                            onChange={this.handleSliderChange}
                            value={currentFrame}
                            trackStyle={{
                                background: '#28C58E'
                            }}
                            railStyle={{
                                background: '#797C8E'
                            }}
                            handleRender={handleRender}
                            onBeforeChange={() => {
                                this.setState({pause: true});
                            }}
                            onAfterChange={() => this.startTimer()}
                        />
                    </div>
                    <div className={styles.footer}>
                        <div className={styles.icons}>
                            {pause || this.state.currentFrame === this.props.replays.length - 1 ? (
                                <PlaySmall
                                    onClick={() => {
                                        if (this.state.currentFrame === this.props.replays.length - 1) {
                                            this.setState({currentFrame: 0});
                                        }
                                        this.setState({pause: false});
                                        this.startTimer();
                                    }}
                                />
                            ) : (
                                <PauseSmall onClick={() => this.setState({pause: true})} />
                            )}
                        </div>
                        <div className={styles.steps}>{`步数 ${currentFrame || '-'} / ${
                            this.props.replays.length - 1
                        }`}</div>
                        <div className={styles.dropdown}>
                            <Space size="middle">
                                <Button
                                    size="small"
                                    type="text"
                                    onClick={() => this.handleMenuClick({key: 20})}
                                    className={classnames(styles.button, {[styles.on]: this.state.fps === 20})}>
                                    20步/秒
                                </Button>
                                <Button
                                    size="small"
                                    type="text"
                                    onClick={() => this.handleMenuClick({key: 10})}
                                    className={classnames(styles.button, {[styles.on]: this.state.fps === 10})}>
                                    10步/秒
                                </Button>
                                <Button
                                    size="small"
                                    onClick={() => this.handleMenuClick({key: 5})}
                                    type="text"
                                    className={classnames(styles.button, {[styles.on]: this.state.fps === 5})}>
                                    5步/秒
                                </Button>
                                <Button
                                    onClick={() => this.handleMenuClick({key: 3})}
                                    size="small"
                                    type="text"
                                    className={classnames(styles.button, {[styles.on]: this.state.fps === 3})}>
                                    3步/秒
                                </Button>
                                <Button
                                    onClick={() => this.handleMenuClick({key: 1})}
                                    size="small"
                                    type="text"
                                    className={classnames(styles.button, {[styles.on]: this.state.fps === 1})}>
                                    1步/秒
                                </Button>
                            </Space>
                        </div>
                        <div className={styles.full}>
                            <VideoFullscreen onClick={this.props.switchFullscreen} />
                        </div>
                    </div>
                </div>
            </div>
        );
    }
}

export default MapReplays;
