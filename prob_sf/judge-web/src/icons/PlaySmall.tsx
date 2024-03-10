import React from 'react';
const PlaySmall = ({onClick}: {onClick: (...args: any) => void}) => (
    <svg
        onClick={onClick}
        width="16px"
        height="16px"
        viewBox="0 0 16 16"
        version="1.1"
        xmlns="http://www.w3.org/2000/svg"
        xmlnsXlink="http://www.w3.org/1999/xlink">
        <g id="页面-1" stroke="none" strokeWidth="1" fill="none" fillRule="evenodd">
            <g id="算法大赛-地图回放-播放" transform="translate(-340.000000, -562.000000)">
                <g id="icon-播放" transform="translate(340.000000, 562.000000)">
                    <rect id="矩形" x="0" y="0" width="16" height="16" />
                    <path
                        d="M13.5731081,6.77927149 L3.33786857,0.655031056 C3.06536821,0.492136158 2.72553679,0.487351064 2.44848265,0.642507801 C2.17142851,0.797664539 2,1.08872904 2,1.40426393 L2,13.6527448 C1.99864616,13.9689534 2.16982592,14.2611695 2.44749067,14.4166297 C2.72515543,14.57209 3.06588669,14.566485 3.33819945,14.4019777 L13.573439,8.27773725 C13.8381726,8.11935486 14,7.83511327 14,7.52850437 C14,7.22189548 13.8381726,6.93765388 13.573439,6.77927149 L13.5731081,6.77927149 Z"
                        id="路径"
                        fill="#FFFFFF"
                        fillRule="nonzero"
                    />
                </g>
            </g>
        </g>
    </svg>
);

export default PlaySmall;
