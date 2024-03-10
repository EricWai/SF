import dayjs from 'dayjs';
import duration from 'dayjs/plugin/duration';
dayjs.extend(duration);

export const formatTimeUsed = (value: number, count = 5) => {
    // 36时24分59.11012秒  131099110.12 test
    const durationTime = dayjs.duration(value);
    const asHours = durationTime.asHours().toFixed(0);
    const minutes = durationTime.minutes();
    const seconds = durationTime.seconds();
    const milliseconds = durationTime.milliseconds();
    // 毫秒转5位整数, 并且四舍五入
    const round5Milliseconds = Math.round(milliseconds * 100);
    // 秒拼接毫秒
    const secondsString = (seconds + round5Milliseconds / (10 * count)).toFixed(count);
    // 输出
    return `${asHours}时${minutes}分\n${secondsString}秒`;
};

export default formatTimeUsed;
