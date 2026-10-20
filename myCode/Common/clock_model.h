/**
 * @file clock_model.h
 * 一个"可以调的时钟"，代替 CanMV 上的硬件 RTC。
 *
 * 原代码用 `machine.RTC()`：
 *     clock_manager = ClockManager(2024, 12, 12, 8, 0, 0, 0)
 *     time = clock_manager.get_time()      # (年,月,日,星期,时,分,秒,微秒)
 *     clock_manager.set_time(...)          # Settings App 里的滚轮改的就是它
 *
 * PC 上没有 RTC，但用户也不希望把电脑的时间改掉，所以这里用"真实时间 + 偏移量"：
 *   - get() 返回 真实本地时间 + offset
 *   - set() 把 offset 重新算成 (目标时间 - 真实时间)
 * 这样锁屏、状态栏、Clock App、Settings App 看到的是同一个时间，改时间也不会影响系统。
 */
#ifndef MYCODE_COMMON_CLOCK_MODEL_H
#define MYCODE_COMMON_CLOCK_MODEL_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 记录基准时间（offset = 0）。在 launcher 初始化时调用一次。 */
void clock_model_init(void);

/** 取当前显示时间（已含用户设置的偏移）。 */
void clock_model_get(struct tm * out);

/** 设定显示时间（year 用 4 位年份，例如 2024）。 */
void clock_model_set(int year, int mon, int day, int hour, int min, int sec);

/** 复位偏移，回到真实系统时间。 */
void clock_model_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_COMMON_CLOCK_MODEL_H */
