const PauseSmall = ({onClick}: {onClick: (...args: any) => void}) => (
    <svg
        onClick={onClick}
        width="16px"
        height="16px"
        viewBox="0 0 16 16"
        version="1.1"
        xmlns="http://www.w3.org/2000/svg"
        xmlnsXlink="http://www.w3.org/1999/xlink">
        <g id="页面-1" stroke="none" strokeWidth="1" fill="none" fillRule="evenodd">
            <g id="算法大赛-地图回放-暂停" transform="translate(-340.000000, -562.000000)">
                <g id="icon-暂停" transform="translate(340.000000, 562.000000)">
                    <g id="icon-播放">
                        <rect id="矩形" x="0" y="0" width="16" height="16" />
                    </g>
                    <rect id="矩形" fill="#FFFFFF" x="2.01704545" y="2" width="3" height="12" />
                    <rect id="矩形备份-13" fill="#FFFFFF" x="11.0170455" y="2" width="3" height="12" />
                </g>
            </g>
        </g>
    </svg>
);

export default PauseSmall;