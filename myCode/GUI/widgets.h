/**
 * @file widgets.h
 * 几个反复用到的小控件/工具函数。
 *
 * 这些函数只是把原 main.py 里到处重复的
 *     obj.set_style_pad_all(0, 0); obj.set_style_border_width(0, 0);
 *     obj.set_style_radius(0, 0); obj.clear_flag(lv.obj.FLAG.SCROLLABLE)
 * 收成一行，后面的界面代码全靠它们。
 */
#ifndef MYCODE_GUI_WIDGETS_H
#define MYCODE_GUI_WIDGETS_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 透明容器：无内边距、无边框、无圆角、不可滚动。 */
lv_obj_t * ui_plain_obj(lv_obj_t * parent, lv_coord_t w, lv_coord_t h);

/** 带竖直渐变的容器，用来做锁屏/主屏背景。 */
lv_obj_t * ui_gradient_bg(lv_obj_t * parent, lv_coord_t w, lv_coord_t h,
                          lv_color_t top, lv_color_t bottom, lv_coord_t radius);

/** 创建一个用指定字体/颜色的文本标签。 */
lv_obj_t * ui_label(lv_obj_t * parent, const lv_font_t * font, lv_color_t color, const char * text);

/** 圆角小条（"home bar" 那根横条）。 */
lv_obj_t * ui_pill(lv_obj_t * parent, lv_coord_t w, lv_coord_t h,
                   lv_color_t color, lv_opa_t opa);

/** 把一张资源里的 PNG 做成 lv_img。 */
lv_obj_t * ui_img(lv_obj_t * parent, const char * asset_rel);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_GUI_WIDGETS_H */
