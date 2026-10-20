/**
 * @file status_bar.h
 * 主屏顶部的状态栏（原 main.py 的 HomeScreen.StatusBar）。
 *
 * 原代码里它只做三件事：
 *   1. 左上角显示 HH:MM
 *   2. 文字可切换黑/白（对应 light_mode() / dark_mode()）
 *   3. 进/出全屏时整条上移隐藏
 */
#ifndef MYCODE_STATUS_BAR_H
#define MYCODE_STATUS_BAR_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct status_bar status_bar_t;

status_bar_t * status_bar_create(lv_obj_t * parent);

void status_bar_set_time(status_bar_t * bar, int hour, int minute);

/** 浅色背景 → 传 UI_COLOR_TEXT_DARK；深色背景 → UI_COLOR_TEXT_LIGHT。 */
void status_bar_set_text_color(status_bar_t * bar, lv_color_t color);

/** 进入全屏：状态栏上移隐藏（复刻原代码 0 → -40 的位移动画）。 */
void status_bar_enter_full_screen(status_bar_t * bar);

/** 退出全屏：状态栏移回来。 */
void status_bar_exit_full_screen(status_bar_t * bar);

lv_obj_t * status_bar_obj(status_bar_t * bar);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_STATUS_BAR_H */
