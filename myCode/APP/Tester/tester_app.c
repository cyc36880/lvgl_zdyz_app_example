/**
 * @file tester_app.c
 *
 * 原 main.py 的 AppTester 用 4 个几乎一样的控件类（LEDRCtrl / LEDBCtrl /
 * BuzzerLoudnessCtrl / BuzzerFrequencyCtrl），每个都是：
 *   - 一个圆角"深色竖槽" + 一个白色挡片，挡片上下移动表示百分比
 *   - 上方一个带阴影的圆形图标，阴影宽度随百分比变化（= 发光强度）
 *   - PRESSING 时按手指的 y 换算百分比
 *
 * 这里把它们合并成一个 pct_ctrl_t，4 个实例只是颜色和标题不同。
 */
#include "myCode/APP/Tester/tester_app.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <stdio.h>

#define BAR_W       96
#define BAR_H       240
#define CTRL_NUM    4

typedef struct {
    lv_obj_t *   bar;
    lv_obj_t *   fill;
    lv_obj_t *   icon;
    lv_obj_t *   value_label;
    lv_color_t   color;
    int          percent;
} pct_ctrl_t;

static void pct_ctrl_update(pct_ctrl_t * c)
{
    lv_coord_t h = lv_obj_get_height(c->bar);
    if(h <= 0) h = BAR_H;

    lv_coord_t fill_h = (lv_coord_t)((int32_t)h * c->percent / 100);
    if(fill_h < 2) fill_h = 2;
    lv_obj_set_height(c->fill, fill_h);

    lv_label_set_text_fmt(c->value_label, "%d%%", c->percent);

    if(c->percent == 0) {
        /* 关掉时图标变暗、没有光晕 */
        lv_obj_set_style_bg_color(c->icon, lv_color_darken(c->color, 160), 0);
        lv_obj_set_style_shadow_width(c->icon, 0, 0);
        lv_obj_set_style_shadow_spread(c->icon, 0, 0);
    }
    else {
        lv_obj_set_style_bg_color(c->icon, c->color, 0);
        lv_obj_set_style_shadow_width(c->icon, (lv_coord_t)(c->percent * 45 / 100), 0);
        lv_obj_set_style_shadow_spread(c->icon, (lv_coord_t)(c->percent * 45 / 100 / 10), 0);
    }
}

static void pct_ctrl_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    pct_ctrl_t *    c    = (pct_ctrl_t *)lv_event_get_user_data(e);
    if(code != LV_EVENT_PRESSING && code != LV_EVENT_PRESSED) return;

    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p);

    lv_area_t ba;
    lv_obj_get_coords(c->bar, &ba);
    lv_coord_t h = ba.y2 - ba.y1;
    if(h <= 0) return;

    int percent = (int)(((int32_t)(ba.y2 - p.y) * 100) / h);
    if(percent < 0)   percent = 0;
    if(percent > 100) percent = 100;

    c->percent = percent;
    pct_ctrl_update(c);
}

static pct_ctrl_t * make_pct_ctrl(lv_obj_t * parent, lv_color_t color,
                                  const char * title, int default_percent)
{
    pct_ctrl_t * c = (pct_ctrl_t *)lv_mem_alloc(sizeof(pct_ctrl_t));
    lv_memset_00(c, sizeof(pct_ctrl_t));
    c->color   = color;
    c->percent = default_percent;

    lv_obj_t * slot = ui_plain_obj(parent, BAR_W + 20, 340);

    /* 图标（会发光） */
    c->icon = ui_pill(slot, 40, 40, color, LV_OPA_COVER);
    lv_obj_align(c->icon, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_shadow_color(c->icon, color, 0);

    /* 标题 */
    lv_obj_t * label = ui_label(slot, UI_FONT_SMALL, lv_color_hex(0x3A3A3A), title);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 52);

    /* 百分比 */
    c->value_label = ui_label(slot, UI_FONT_NORMAL, lv_color_hex(0x000000), "0%");
    lv_obj_align(c->value_label, LV_ALIGN_TOP_MID, 0, 78);

    /* 竖槽 */
    c->bar = ui_plain_obj(slot, BAR_W, BAR_H);
    lv_obj_align(c->bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(c->bar, 40, 0);
    lv_obj_set_style_clip_corner(c->bar, true, 0);
    lv_obj_set_style_bg_color(c->bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(c->bar, 120, 0);

    /* 白色填充（从底部往上长） */
    c->fill = ui_plain_obj(c->bar, lv_pct(100), BAR_H);
    lv_obj_align(c->fill, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(c->fill, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(c->fill, LV_OPA_COVER, 0);

    /* 整个槽负责收手指事件 */
    lv_obj_add_flag(c->bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(c->bar, pct_ctrl_event_cb, LV_EVENT_PRESSED, c);
    lv_obj_add_event_cb(c->bar, pct_ctrl_event_cb, LV_EVENT_PRESSING, c);

    pct_ctrl_update(c);
    return c;
}

lv_obj_t * tester_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0xF7F7F9), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    lv_obj_t * title = ui_label(page, UI_FONT_TITLE, lv_color_hex(0x000000), "Tester");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 30, 45);

    lv_obj_t * hint = ui_label(page, UI_FONT_SMALL, lv_color_hex(0x9A9A9A),
                               "Drag the bars up / down");
    lv_obj_align(hint, LV_ALIGN_TOP_LEFT, 30, 82);

    /* 4 列 */
    lv_obj_t * row = ui_plain_obj(page, lv_pct(100), 350);
    lv_obj_align(row, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    make_pct_ctrl(row, lv_color_hex(0xFF0000), "LED R",  50);
    make_pct_ctrl(row, lv_color_hex(0x0000FF), "LED B",  50);
    make_pct_ctrl(row, lv_color_hex(0xF99429), "Buzzer", 50);
    make_pct_ctrl(row, lv_color_hex(0x4CAF50), "Volume", 30);

    return page;
}
