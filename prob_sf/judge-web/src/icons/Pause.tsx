
import React from 'react';
const Pause = ({onClick}: {onClick: (...args: any) => void}) => (
    <svg
        onClick={onClick}
        width="42px"
        height="42px"
        viewBox="0 0 42 42"
        version="1.1"
        xmlns="http://www.w3.org/2000/svg"
        xmlnsXlink="http://www.w3.org/1999/xlink">
        <g id="页面-1" stroke="none" strokeWidth="1" fill="none" fillRule="evenodd">
            <g id="算法大赛-地图回放-暂停" transform="translate(-664.000000, -385.000000)">
                <g id="icon-暂停" transform="translate(664.000000, 385.000000)">
                    <g id="icon-播放">
                        <rect id="矩形" x="0" y="0" width="42" height="42" />
                    </g>
                    <rect id="矩形" fill="#FFFFFF" x="5" y="5" width="9" height="32" />
                    <rect id="矩形备份-13" fill="#FFFFFF" x="28" y="5" width="9" height="32" />
                </g>
            </g>
        </g>
    </svg>
);

export default Pause;
