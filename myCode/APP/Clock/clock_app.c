/**
 * @file clock_app.c
 *
 * 原 main.py 的 AppClock 有两个子模块：
 *   - TopBar.LocalClock：DialHand 用 lv_line 画 6 段线（时/分/秒各两段：中心短的一段 + 外圈长的一段）
 *   - TopBar.Stopwatch：秒表，靠一个 lv.timer 按 100ms 刷新
 * 这里把它们合成一个文件，用一个共用的 100ms timer 驱动当前显示的那个页面。
 *
 * 指针坐标的算法和原代码一致：角度 0° 指向 12 点，顺时针增加，
 *   x = cx + sin(angle) * len
 *   y = cy - cos(angle) * len
 */
#include "myCode/APP/Clock/clock_app.h"
#include "myCode/Common/clock_model.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <math.h>
#include <stdio.h>

#define PI_D 3.14159265358979

#define DIAL_SIZE   240
#define DIAL_R      (DIAL_SIZE / 2)
#define NUM_R       95

typedef enum { TAB_CLOCK = 0, TAB_STOPWATCH } clock_tab_t;

typedef struct {
    lv_obj_t *   page;
    clock_tab_t  tab;

    /* Clock 页 */
    lv_obj_t *   clock_view;
    lv_obj_t *   dial;
    lv_obj_t *   time_label;
    lv_obj_t *   hour_hand;
    lv_obj_t *   min_hand;
    lv_obj_t *   sec_hand;
    lv_obj_t *   sec_tail;
    lv_point_t   hour_pts[2];
    lv_point_t   min_pts[2];
    lv_point_t   sec_pts[2];
    lv_point_t   tail_pts[2];

    /* Stopwatch 页 */
    lv_obj_t *   watch_view;
    lv_obj_t *   watch_label;
    lv_obj_t *   watch_start_btn;
    lv_obj_t *   watch_start_label;
    uint32_t     watch_accum_ms;   /* 已经累计的毫秒 */
    uint32_t     watch_start_tick; /* 本次启动的时刻 */
    bool         watch_running;

    /* 顶部标签 */
    lv_obj_t *   tab_label[2];

    lv_timer_t * timer;
} clock_app_t;

/* ---------------- 指针 ---------------- */

static void hand_set(lv_obj_t * line, lv_point_t * pts, lv_coord_t cx, lv_coord_t cy,
                     double deg, double len, lv_coord_t width, lv_color_t color)
{
    double rad = deg * PI_D / 180.0;
    pts[0].x = cx;
    pts[0].y = cy;
    pts[1].x = cx + (lv_coord_t)(sin(rad) * len);
    pts[1].y = cy - (lv_coord_t)(cos(rad) * len);
    lv_line_set_points(line, pts, 2);
    lv_obj_set_style_line_width(line, width, 0);
    lv_obj_set_style_line_color(line, color, 0);
    lv_obj_set_style_line_rounded(line, true, 0);
}

static void clock_view_update(clock_app_t * app)
{
    struct tm tmv;
    clock_model_get(&tmv);

    double sec  = (double)tmv.tm_sec;
    double min  = (double)tmv.tm_min + sec / 60.0;
    double hour = (double)(tmv.tm_hour % 12) + min / 60.0;

    hand_set(app->hour_hand, app->hour_pts, DIAL_R, DIAL_R, hour * 30.0, 60, 8,
             lv_color_hex(0x000000));
    hand_set(app->min_hand, app->min_pts, DIAL_R, DIAL_R, min * 6.0, 100, 8,
             lv_color_hex(0x000000));
    hand_set(app->sec_hand, app->sec_pts, DIAL_R, DIAL_R, sec * 6.0, 110, 3,
             lv_color_hex(0xEF8732));
    /* 秒针的尾巴（指向反方向） */
    hand_set(app->sec_tail, app->tail_pts, DIAL_R, DIAL_R, sec * 6.0 + 180.0, 20, 3,
             lv_color_hex(0xEF8732));

    lv_label_set_text_fmt(app->time_label, "%02d:%02d:%02d", tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
}

/* ---------------- 秒表 ---------------- */

static void watch_refresh(clock_app_t * app)
{
    uint32_t ms = app->watch_accum_ms;
    if(app->watch_running) ms += lv_tick_elaps(app->watch_start_tick);

    lv_label_set_text_fmt(app->watch_label, "%02u:%02u.%02u",
                          (unsigned)(ms / 60000u),
                          (unsigned)((ms / 1000u) % 60u),
                          (unsigned)((ms / 10u) % 100u));
}

static void watch_start_pause(clock_app_t * app)
{
    if(app->watch_running) {
        app->watch_accum_ms += lv_tick_elaps(app->watch_start_tick);
        app->watch_running = false;
        lv_label_set_text(app->watch_start_label, "Start");
    }
    else {
        app->watch_start_tick = lv_tick_get();
        app->watch_running = true;
        lv_label_set_text(app->watch_start_label, "Pause");
    }
    watch_refresh(app);
}

/* ---------------- 顶部标签 ---------------- */

static void apply_tab(clock_app_t * app, clock_tab_t tab)
{
    app->tab = tab;
    if(tab == TAB_CLOCK) {
        lv_obj_clear_flag(app->clock_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(app->watch_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(app->tab_label[0], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_color(app->tab_label[1], lv_color_hex(0x7F7F7F), 0);
        clock_view_update(app);
    }
    else {
        lv_obj_add_flag(app->clock_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(app->watch_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(app->tab_label[0], lv_color_hex(0x7F7F7F), 0);
        lv_obj_set_style_text_color(app->tab_label[1], lv_color_hex(0xFFFFFF), 0);
        watch_refresh(app);
    }
}

static void tab_event_cb(lv_event_t * e)
{
    clock_app_t * app = (clock_app_t *)lv_event_get_user_data(e);
    lv_obj_t *    obj = lv_event_get_target(e);
    apply_tab(app, (obj == app->tab_label[1]) ? TAB_STOPWATCH : TAB_CLOCK);
}

static void watch_btn_event_cb(lv_event_t * e)
{
    clock_app_t * app = (clock_app_t *)lv_event_get_user_data(e);
    lv_obj_t *    btn = lv_event_get_target(e);

    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    if(btn == app->watch_start_btn) {
        watch_start_pause(app);
    }
    else {                                                     /* Reset */
        app->watch_accum_ms = 0;
        app->watch_running  = false;
        lv_label_set_text(app->watch_start_label, "Start");
        watch_refresh(app);
    }
}

/* ---------------- 定时器 ---------------- */

static void app_timer_cb(lv_timer_t * t)
{
    clock_app_t * app = (clock_app_t *)t->user_data;
    if(app == NULL || app->page == NULL) return;

    if(app->tab == TAB_CLOCK) clock_view_update(app);
    else if(app->watch_running) watch_refresh(app);
}

static void page_deleted_cb(lv_event_t * e)
{
    clock_app_t * app = (clock_app_t *)lv_event_get_user_data(e);
    if(app == NULL) return;
    if(app->timer) {
        lv_timer_del(app->timer);
        app->timer = NULL;
    }
    lv_mem_free(app);
}

/* ---------------- 创建 ---------------- */

static lv_obj_t * make_btn(lv_obj_t * parent, const char * text, lv_obj_t ** out_label)
{
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_set_style_radius(btn, 24, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2C2C34), 0);

    lv_obj_t * label = ui_label(btn, UI_FONT_NORMAL, lv_color_hex(0xFFFFFF), text);
    lv_obj_center(label);
    if(out_label) *out_label = label;
    return btn;
}

lv_obj_t * clock_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;

    clock_app_t * app = (clock_app_t *)lv_mem_alloc(sizeof(clock_app_t));
    lv_memset_00(app, sizeof(clock_app_t));

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    app->page = page;

    /* ---- 顶部两个标签 ---- */
    app->tab_label[0] = ui_label(page, UI_FONT_TITLE, lv_color_hex(0xFFFFFF), "Clock");
    lv_obj_align(app->tab_label[0], LV_ALIGN_TOP_LEFT, 30, 55);
    app->tab_label[1] = ui_label(page, UI_FONT_TITLE, lv_color_hex(0x7F7F7F), "Stopwatch");
    lv_obj_align(app->tab_label[1], LV_ALIGN_TOP_LEFT, 150, 55);

    for(int i = 0; i < 2; i++) {
        lv_obj_add_flag(app->tab_label[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(app->tab_label[i], tab_event_cb, LV_EVENT_CLICKED, app);
    }

    /* ---- Clock 页：表盘 ---- */
    app->clock_view = ui_plain_obj(page, lv_pct(100), lv_pct(100));
    lv_obj_align(app->clock_view, LV_ALIGN_TOP_LEFT, 0, 0);

    app->dial = lv_obj_create(app->clock_view);
    lv_obj_set_size(app->dial, DIAL_SIZE, DIAL_SIZE);
    lv_obj_align(app->dial, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_pad_all(app->dial, 0, 0);
    lv_obj_set_style_border_width(app->dial, 0, 0);
    lv_obj_set_style_radius(app->dial, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(app->dial, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(app->dial, LV_OPA_COVER, 0);
    lv_obj_clear_flag(app->dial, LV_OBJ_FLAG_SCROLLABLE);

    /* 表盘上的 1..12 */
    for(int i = 1; i <= 12; i++) {
        double rad = (double)i * 30.0 * PI_D / 180.0;
        lv_coord_t dx = (lv_coord_t)(sin(rad) * NUM_R);
        lv_coord_t dy = (lv_coord_t)(-cos(rad) * NUM_R);

        lv_obj_t * num = ui_label(app->dial, UI_FONT_TITLE, lv_color_hex(0x000000), "");
        lv_label_set_text_fmt(num, "%d", i);
        lv_obj_align(num, LV_ALIGN_CENTER, dx, dy);
    }

    /* 指针：lv_line 的坐标是"父对象(dial)坐标系"，所以圆心是 (DIAL_R, DIAL_R) */
    app->hour_hand = lv_line_create(app->dial);
    app->min_hand  = lv_line_create(app->dial);
    app->sec_hand  = lv_line_create(app->dial);
    app->sec_tail  = lv_line_create(app->dial);

    lv_obj_t * center_dot = ui_pill(app->dial, 12, 12, lv_color_hex(0x000000), LV_OPA_COVER);
    lv_obj_align(center_dot, LV_ALIGN_CENTER, 0, 0);

    /* 表盘下面的数字时间 */
    app->time_label = ui_label(app->clock_view, UI_FONT_HUGE, lv_color_hex(0xFFFFFF), "00:00:00");
    lv_obj_align_to(app->time_label, app->dial, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    /* ---- Stopwatch 页 ---- */
    app->watch_view = ui_plain_obj(page, lv_pct(100), lv_pct(100));
    lv_obj_align(app->watch_view, LV_ALIGN_TOP_LEFT, 0, 0);

    app->watch_label = ui_label(app->watch_view, UI_FONT_HUGE, lv_color_hex(0xFFFFFF), "00:00.00");
    lv_obj_align(app->watch_label, LV_ALIGN_CENTER, 0, -70);

    lv_obj_t * reset_btn = make_btn(app->watch_view, "Reset", NULL);
    lv_obj_set_size(reset_btn, 150, 70);
    lv_obj_align(reset_btn, LV_ALIGN_CENTER, -95, 60);

    lv_obj_t * start_btn = make_btn(app->watch_view, "Start", &app->watch_start_label);
    lv_obj_set_size(start_btn, 150, 70);
    lv_obj_align(start_btn, LV_ALIGN_CENTER, 95, 60);
    lv_obj_set_style_bg_color(start_btn, lv_color_hex(0xF99429), 0);
    app->watch_start_btn = start_btn;

    lv_obj_add_event_cb(reset_btn, watch_btn_event_cb, LV_EVENT_CLICKED, app);
    lv_obj_add_event_cb(start_btn, watch_btn_event_cb, LV_EVENT_CLICKED, app);

    /* ---- 启动 ---- */
    apply_tab(app, TAB_CLOCK);

    app->timer = lv_timer_create(app_timer_cb, 100, app);
    lv_obj_add_event_cb(page, page_deleted_cb, LV_EVENT_DELETE, app);

    return page;
}
