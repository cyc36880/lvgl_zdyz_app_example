/**
 * @file theme.h
 * 界面的公共常量（尺寸、颜色、字体）。
 *
 * 原 MicroPython 代码里这些值散落在各处（lv.color_hex(0xFFFFFF)、lv.pct(50) 等），
 * 移植到 C 之后集中放在这里，方便你对照原代码、也方便统一改配色。
 */
#ifndef MYCODE_THEME_H
#define MYCODE_THEME_H

#include "lvgl/lvgl.h"

/* ---------- 屏幕尺寸：和 CanMV 那份 640x480 的 ST7701 屏一致 ---------- */
#define UI_W 640
#define UI_H 480

/* ---------- 配色 ----------
 * 锁屏和主屏都用同一条「上白下黑」的竖直渐变，这是原代码的设计：
 *   conv.set_style_bg_color(0xFFFFFF) + bg_grad_color(0x000000) + GRAD_DIR.VER
 * 所以上方的图标/文字用黑色，越往下越要靠白色才看得清。 */
#define UI_COLOR_BG_TOP      lv_color_hex(0xFFFFFF)
#define UI_COLOR_BG_BOTTOM   lv_color_hex(0x000000)
#define UI_COLOR_TEXT_DARK   lv_color_hex(0x000000)
#define UI_COLOR_TEXT_LIGHT  lv_color_hex(0xFFFFFF)
#define UI_COLOR_ACCENT      lv_color_hex(0x2196F3)
#define UI_COLOR_DANGER      lv_color_hex(0xF44336)
#define UI_COLOR_MASK        lv_color_hex(0x000000)

/* ---------- 字体 ----------
 * PC 版 LVGL v8 没有运行时字库加载（MicroPython 用 lv.font_load 读 .bin），
 * 所以用 lv_conf.h 里内置的 Montserrat 字号的字体。 */
#define UI_FONT_SMALL   (&lv_font_montserrat_16)
#define UI_FONT_NORMAL  (&lv_font_montserrat_20)
#define UI_FONT_TITLE   (&lv_font_montserrat_28)
#define UI_FONT_HUGE    (&lv_font_montserrat_48)

/* ---------- 尺寸 ---------- */
#define UI_STATUS_BAR_H      40   /* 状态栏高度，全屏时上移同样的距离 */
#define UI_DOCK_H            130  /* 底部 Dock 高度 */
#define UI_ICON_SLOT_W       120
#define UI_ICON_SLOT_H       132
#define UI_ICON_IMG_SIZE     90   /* 图标本身 90x90 */
#define UI_SCREEN_RADIUS     20   /* 外层圆角（模拟设备圆角屏） */
#define UI_APP_ANIM_TIME     200  /* 打开/关闭 App 的动画时长 */
#define UI_APP_DRAG_H        40   /* App 底部"上滑关闭"热区高度 */

/* ---------- 动画时长（对应原代码里 set_time() 的那些数字） ---------- */
#define UI_ANIM_UNLOCK       300
#define UI_ANIM_LOCK_BACK    300

#endif /* MYCODE_THEME_H */
