import {Tooltip} from 'antd';
import type {SliderProps} from 'rc-slider';
import raf from 'rc-util/lib/raf';
import React from 'react';

const HandleTooltip = (props: {
    value: number;
    children: React.ReactElement;
    visible: boolean;
    tipFormatter?: (value: number) => React.ReactNode;
}) => {
    const {value, children, visible, tipFormatter = val => `${val} %`, ...restProps} = props;

    const tooltipRef = React.useRef<any>();
    const rafRef = React.useRef<number | null>(null);

    function cancelKeepAlign() {
        raf.cancel(rafRef.current!);
    }

    function keepAlign() {
        rafRef.current = raf(() => {
            tooltipRef.current?.forcePopupAlign();
        });
    }

    React.useEffect(() => {
        if (visible) {
            keepAlign();
        } else {
            cancelKeepAlign();
        }

        return cancelKeepAlign;
    }, [value, visible]);

    return (
        <Tooltip
            placement="top"
            overlay={tipFormatter(value)}
            overlayInnerStyle={{minHeight: 'auto'}}
            ref={tooltipRef}
            open={visible}
            {...restProps}>
            {children}
        </Tooltip>
    );
};

export default HandleTooltip;

export const handleRender: SliderProps['handleRender'] = (node, props) => {
    return (
        <HandleTooltip value={props.value} tipFormatter={v => '第' + (v + 1) + '步'} visible={props.dragging}>
            {node}
        </HandleTooltip>
    );
};
