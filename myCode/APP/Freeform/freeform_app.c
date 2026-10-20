/**
 * @file freeform_app.c
 *
 * 原 main.py 的 AppFreeform：
 *   - 整屏一个 lv.canvas，PRESSED/PRESSING 时把"上一个点 → 当前点"用 lv.draw_line 画上去
 *   - 左边缘有个 50x30% 的透明手柄，向左/右拖动可以把画笔设置面板拉出来
 *   - 面板里有：颜色预览圆点、R/G/B 三个滑块、粗细滑块、Clean 按钮
 *
 * 这里保留同样的画板和设置项；面板改成"点手柄开关"（带 300ms 动画），
 * 比拖拽更不容易误触，代码也短很多（原版的拖动逻辑可以自己试着加回来）。
 */
#include "myCode/APP/Freeform/freeform_app.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <stdio.h>

#define PANEL_W 260

typedef struct {
    lv_obj_t *        page;
    lv_obj_t *        canvas;
    lv_color_t *      canvas_buf;

    lv_obj_t *        panel;
    bool              panel_open;

    lv_obj_t *        pen_dot;       /* 颜色预览圆点 */
    lv_obj_t *        slider_r;
    lv_obj_t *        slider_g;
    lv_obj_t *        slider_b;
    lv_obj_t *        slider_size;

    lv_draw_line_dsc_t line_dsc;
    lv_point_t        prev_pt;
    bool              has_prev;
} freeform_t;

/* ---------------- 画线 ---------------- */

static void draw_segment(freeform_t * f, lv_point_t from, lv_point_t to)
{
    lv_point_t pts[2] = { from, to };

    lv_canvas_draw_line(f->canvas, pts, 2, &f->line_dsc);

    /* 只刷新这一小片区域 */
    lv_coord_t pad = (lv_coord_t)f->line_dsc.width;
    lv_area_t area;
    area.x1 = (from.x < to.x ? from.x : to.x) - pad;
    area.y1 = (from.y < to.y ? from.y : to.y) - pad;
    area.x2 = (from.x > to.x ? from.x : to.x) + pad;
    area.y2 = (from.y > to.y ? from.y : to.y) + pad;
    lv_obj_invalidate_area(f->canvas, &area);
}

static void canvas_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    freeform_t *    f    = (freeform_t *)lv_event_get_user_data(e);

    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p);

    /* 转换到 canvas 自己的坐标系 */
    lv_area_t ca;
    lv_obj_get_coords(f->canvas, &ca);
    p.x -= ca.x1;
    p.y -= ca.y1;

    if(code == LV_EVENT_PRESSED) {
        f->prev_pt  = p;
        f->has_prev = true;
        /* 点一下也画一个点（用一条极短的线表示） */
        lv_point_t p2 = { p.x + 1, p.y + 1 };
        draw_segment(f, p, p2);
    }
    else if(code == LV_EVENT_PRESSING) {
        if(!f->has_prev) {
            f->prev_pt  = p;
            f->has_prev = true;
            return;
        }
        draw_segment(f, f->prev_pt, p);
        f->prev_pt = p;
    }
    else if(code == LV_EVENT_RELEASED) {
        f->has_prev = false;
    }
}

/* ---------------- 设置面板 ---------------- */

static void panel_x_anim_cb(void * var, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)var, v);
}

static void panel_animate(freeform_t * f, bool open)
{
    f->panel_open = open;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, f->panel);
    lv_anim_set_values(&a, lv_obj_get_x(f->panel), open ? 0 : -PANEL_W);
    lv_anim_set_time(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, panel_x_anim_cb);
    lv_anim_start(&a);
}

static void handle_event_cb(lv_event_t * e)
{
    freeform_t * f = (freeform_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        panel_animate(f, !f->panel_open);
    }
}

static void color_slider_event_cb(lv_event_t * e)
{
    freeform_t * f = (freeform_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

    lv_color_t color = lv_color_make((uint8_t)lv_slider_get_value(f->slider_r),
                                     (uint8_t)lv_slider_get_value(f->slider_g),
                                     (uint8_t)lv_slider_get_value(f->slider_b));
    f->line_dsc.color = color;
    lv_obj_set_style_bg_color(f->pen_dot, color, 0);
}

static void size_slider_event_cb(lv_event_t * e)
{
    freeform_t * f = (freeform_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

    f->line_dsc.width = (uint8_t)lv_slider_get_value(f->slider_size);
    lv_obj_set_size(f->pen_dot, f->line_dsc.width, f->line_dsc.width);
}

static void clean_btn_event_cb(lv_event_t * e)
{
    freeform_t * f = (freeform_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_canvas_fill_bg(f->canvas, lv_color_hex(0xFFFFFF), LV_OPA_COVER);
}

static lv_obj_t * make_color_slider(lv_obj_t * parent, lv_obj_t * align_to,
                                    lv_color_t color, int value)
{
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_set_size(slider, lv_pct(75), 20);
    if(align_to) lv_obj_align_to(slider, align_to, LV_ALIGN_OUT_BOTTOM_MID, 0, 25);
    else         lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 170);

    lv_obj_set_style_bg_color(slider, color, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_slider_set_range(slider, 0, 255);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    return slider;
}

/* ---------------- 创建 ---------------- */

static void page_deleted_cb(lv_event_t * e)
{
    freeform_t * f = (freeform_t *)lv_event_get_user_data(e);
    if(f == NULL) return;
    if(f->canvas_buf) {
        lv_mem_free(f->canvas_buf);
        f->canvas_buf = NULL;
    }
    lv_mem_free(f);
}

lv_obj_t * freeform_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;

    lv_coord_t w = lv_obj_get_width(parent);
    lv_coord_t h = lv_obj_get_height(parent);
    if(w <= 0) w = 640;
    if(h <= 0) h = 480;

    freeform_t * f = (freeform_t *)lv_mem_alloc(sizeof(freeform_t));
    lv_memset_00(f, sizeof(freeform_t));

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    f->page = page;

    /* ---- 画板 ---- */
    f->canvas_buf = (lv_color_t *)lv_mem_alloc((uint32_t)w * (uint32_t)h * sizeof(lv_color_t));
    f->canvas = lv_canvas_create(page);
    lv_canvas_set_buffer(f->canvas, f->canvas_buf, w, h, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(f->canvas, lv_color_hex(0xFFFFFF), LV_OPA_COVER);
    lv_obj_center(f->canvas);
    lv_obj_clear_flag(f->canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(f->canvas, LV_OBJ_FLAG_CLICKABLE);

    lv_draw_line_dsc_init(&f->line_dsc);
    f->line_dsc.color       = lv_color_make(0xFF, 0x00, 0x00);
    f->line_dsc.width       = 30;
    f->line_dsc.round_end   = 1;
    f->line_dsc.round_start = 1;

    lv_obj_add_event_cb(f->canvas, canvas_event_cb, LV_EVENT_PRESSED, f);
    lv_obj_add_event_cb(f->canvas, canvas_event_cb, LV_EVENT_PRESSING, f);
    lv_obj_add_event_cb(f->canvas, canvas_event_cb, LV_EVENT_RELEASED, f);

    /* ---- 左边缘手柄 ---- */
    lv_obj_t * handle = ui_plain_obj(page, 50, lv_pct(30));
    lv_obj_align(handle, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(handle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(handle, handle_event_cb, LV_EVENT_CLICKED, f);

    lv_obj_t * bar = ui_pill(handle, 12, lv_pct(50), lv_color_hex(0x1F1F1F), LV_OPA_COVER);
    lv_obj_align(bar, LV_ALIGN_LEFT_MID, 10, 0);

    /* ---- 设置面板（默认藏在左边外面） ---- */
    f->panel = lv_obj_create(page);
    lv_obj_set_size(f->panel, PANEL_W, lv_pct(100));
    lv_obj_align(f->panel, LV_ALIGN_LEFT_MID, -PANEL_W, 0);
    lv_obj_set_style_pad_all(f->panel, 0, 0);
    lv_obj_set_style_border_width(f->panel, 0, 0);
    lv_obj_set_style_radius(f->panel, 20, 0);
    lv_obj_set_style_bg_color(f->panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(f->panel, 180, 0);
    lv_obj_clear_flag(f->panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 颜色预览圆点 */
    f->pen_dot = ui_pill(f->panel, f->line_dsc.width, f->line_dsc.width,
                         f->line_dsc.color, LV_OPA_COVER);
    lv_obj_align(f->pen_dot, LV_ALIGN_TOP_MID, 0, 90);

    /* R / G / B / 粗细 */
    f->slider_r = make_color_slider(f->panel, NULL, lv_color_hex(0xFF0000), 0xFF);
    f->slider_g = make_color_slider(f->panel, f->slider_r, lv_color_hex(0x00FF00), 0x00);
    f->slider_b = make_color_slider(f->panel, f->slider_g, lv_color_hex(0x0000FF), 0x00);
    f->slider_size = make_color_slider(f->panel, f->slider_b, lv_color_hex(0x7F7F7F), 30);
    lv_slider_set_range(f->slider_size, 8, 100);

    lv_obj_add_event_cb(f->slider_r, color_slider_event_cb, LV_EVENT_VALUE_CHANGED, f);
    lv_obj_add_event_cb(f->slider_g, color_slider_event_cb, LV_EVENT_VALUE_CHANGED, f);
    lv_obj_add_event_cb(f->slider_b, color_slider_event_cb, LV_EVENT_VALUE_CHANGED, f);
    lv_obj_add_event_cb(f->slider_size, size_slider_event_cb, LV_EVENT_VALUE_CHANGED, f);

    /* Clean 按钮 */
    lv_obj_t * clean_btn = lv_btn_create(f->panel);
    lv_obj_set_size(clean_btn, lv_pct(75), 60);
    lv_obj_align_to(clean_btn, f->slider_size, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
    lv_obj_set_style_radius(clean_btn, 12, 0);
    lv_obj_set_style_bg_color(clean_btn, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_shadow_width(clean_btn, 0, 0);
    lv_obj_set_style_border_width(clean_btn, 0, 0);
    lv_obj_add_event_cb(clean_btn, clean_btn_event_cb, LV_EVENT_CLICKED, f);

    lv_obj_t * clean_label = ui_label(clean_btn, UI_FONT_NORMAL, lv_color_hex(0xFFFFFF), "Clean");
    lv_obj_center(clean_label);

    lv_obj_add_event_cb(page, page_deleted_cb, LV_EVENT_DELETE, f);

    return page;
}
