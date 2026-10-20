/**
 * @file big_clock.h
 * 锁屏大号数字时钟。
 *
 * 原 main.py 用 lv.font_load() 在运行时加载 150px 的 bin 字库：
 *     self.lv_font_lock_screen_clock_time = lv.font_load("A:" + RESOURCES_PATH +
 *         "Fonts/lv_font_lock_screen_clock_time_size150_bpp4.bin")
 * PC 版 LVGL 8 没有运行时字库加载，内置 Montserrat 最大只有 48px，
 * 所以这里改成用 11 张预渲染的位图（Fonts/clock_digit_*.png）拼出 HH:MM。
 * 生成脚本见 tools/gen_clock_digits.py。
 */
#ifndef MYCODE_GUI_BIG_CLOCK_H
#define MYCODE_GUI_BIG_CLOCK_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct big_clock big_clock_t;

/** 创建大时钟（容器是横向 flex，5 个 lv_img：时十位、时个位、冒号、分十位、分个位）。 */
big_clock_t * big_clock_create(lv_obj_t * parent);

/** 返回容器对象，方便外部 align/移动。 */
lv_obj_t * big_clock_obj(big_clock_t * clock);

/** 更新显示时间，hour 0~23，minute 0~59。 */
void big_clock_set_time(big_clock_t * clock, int hour, int minute);

/** 上色（位图本身是白色的，靠 recolor 变成黑/白）。 */
void big_clock_set_color(big_clock_t * clock, lv_color_t color);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_GUI_BIG_CLOCK_H */
