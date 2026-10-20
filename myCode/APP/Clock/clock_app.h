/**
 * @file clock_app.h
 * 时钟 App（对应原 main.py 的 AppClock）：顶部两个标签切换「Clock / Stopwatch」。
 *   - Clock：白色表盘 + 1..12 刻度 + 时/分/秒针 + 下面一行大号数字时间
 *   - Stopwatch：秒表，Start/Pause + Reset
 */
#ifndef MYCODE_APP_CLOCK_H
#define MYCODE_APP_CLOCK_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * clock_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_CLOCK_H */
