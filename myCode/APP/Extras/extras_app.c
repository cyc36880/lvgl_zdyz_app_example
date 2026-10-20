/**
 * @file extras_app.c
 * 主屏第 1 页那 8 个"原版没实现"的 App，各一个小示例页。
 * 风格统一：浅灰底 + 左上角标题 + 内容卡片。
 */
#include "myCode/APP/Extras/extras_app.h"
#include "myCode/Common/clock_model.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <stdio.h>

#define EXTRA_BG    lv_color_hex(0xF7F7F9)
#define CARD_BG     lv_color_hex(0xFFFFFF)
#define TEXT_MAIN   lv_color_hex(0x000000)
#define TEXT_SUB    lv_color_hex(0x8A8A8A)

/* 统一的小工具：页面 + 标题 + 返回一个内容容器 */
static lv_obj_t * page_create(lv_obj_t * parent, const char * title, const char * subtitle)
{
    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, EXTRA_BG, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    lv_obj_t * t = ui_label(page, UI_FONT_TITLE, TEXT_MAIN, title);
    lv_obj_align(t, LV_ALIGN_TOP_LEFT, 30, 45);

    if(subtitle) {
        lv_obj_t * s = ui_label(page, UI_FONT_SMALL, TEXT_SUB, subtitle);
        lv_obj_align(s, LV_ALIGN_TOP_LEFT, 30, 82);
    }
    return page;
}

static lv_obj_t * card(lv_obj_t * parent, lv_coord_t w, lv_coord_t h, lv_obj_t * align_to,
                       lv_coord_t y, lv_color_t color)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    if(align_to == NULL) lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, y);
    else                 lv_obj_align_to(obj, align_to, LV_ALIGN_OUT_BOTTOM_MID, 0, y);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 16, 0);
    lv_obj_set_style_clip_corner(obj, true, 0);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

/* ===================== Books ===================== */

lv_obj_t * books_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Books", "3 items");

    static const char * const titles[] = { "LVGL in Action", "Embedded GUI", "MicroPython Notes" };
    static const char * const authors[] = { "A. Author", "B. Builder", "C. Coder" };
    static const uint32_t cover_color[] = { 0x4C6FFF, 0xFF8A3D, 0x2FBF71 };

    lv_obj_t * prev = NULL;
    for(int i = 0; i < 3; i++) {
        lv_obj_t * row = card(page, lv_pct(88), 100, prev, prev ? 12 : 120, CARD_BG);
        prev = row;

        lv_obj_t * cover = ui_pill(row, 64, 88, lv_color_hex(cover_color[i]), LV_OPA_COVER);
        lv_obj_set_style_radius(cover, 8, 0);
        lv_obj_align(cover, LV_ALIGN_LEFT_MID, 10, 0);

        lv_obj_t * t = ui_label(row, UI_FONT_NORMAL, TEXT_MAIN, titles[i]);
        lv_obj_align_to(t, cover, LV_ALIGN_OUT_RIGHT_MID, 14, -14);

        lv_obj_t * a = ui_label(row, UI_FONT_SMALL, TEXT_SUB, authors[i]);
        lv_obj_align_to(a, t, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
    }
    return page;
}

/* ===================== Files ===================== */

lv_obj_t * files_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Files", "sdcard");

    static const char * const names[] = { "CanMV Sample", "myCode", "Photos", "notes.txt" };
    static const char * const sizes[] = { "4 items", "18 items", "7 items", "2 KB" };
    static const uint32_t colors[] = { 0xFFC94D, 0x4C6FFF, 0x2FBF71, 0x9AA0A6 };

    lv_obj_t * prev = NULL;
    for(int i = 0; i < 4; i++) {
        lv_obj_t * row = card(page, lv_pct(88), 72, prev, prev ? 8 : 118, CARD_BG);
        prev = row;

        lv_obj_t * icon = ui_pill(row, 44, 44, lv_color_hex(colors[i]), LV_OPA_COVER);
        lv_obj_set_style_radius(icon, 12, 0);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

        lv_obj_t * n = ui_label(row, UI_FONT_NORMAL, TEXT_MAIN, names[i]);
        lv_obj_align_to(n, icon, LV_ALIGN_OUT_RIGHT_MID, 14, -8);

        lv_obj_t * sz = ui_label(row, UI_FONT_SMALL, TEXT_SUB, sizes[i]);
        lv_obj_align_to(sz, n, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }
    return page;
}

/* ===================== Calendar ===================== */

lv_obj_t * calendar_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;

    static const char * const month_name[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    struct tm tmv;
    clock_model_get(&tmv);
    int year  = tmv.tm_year + 1900;
    int month = tmv.tm_mon;          /* 0 ~ 11 */
    int today = tmv.tm_mday;

    char title[32];
    snprintf(title, sizeof(title), "%s %d", month_name[month], year);

    lv_obj_t * page = page_create(parent, title, "Calendar");

    /* 这个月 1 号是星期几（tm_wday: 0 = Sunday） */
    struct tm first = tmv;
    first.tm_mday = 1;
    first.tm_hour = 12;
    first.tm_min = 0;
    first.tm_sec = 0;
    time_t t = mktime(&first);
    struct tm first_tm;
#ifdef _WIN32
    localtime_s(&first_tm, &t);
#else
    first_tm = *localtime(&t);
#endif
    int first_wday = first_tm.tm_wday;

    /* 每月天数 */
    static const int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int days = mdays[month];
    if(month == 1 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) days = 29;

    lv_obj_t * grid = card(page, lv_pct(88), 300, NULL, 120, CARD_BG);

    /* 星期表头 */
    static const char * const wd[] = { "S", "M", "T", "W", "T", "F", "S" };
    for(int i = 0; i < 7; i++) {
        lv_obj_t * l = ui_label(grid, UI_FONT_SMALL, TEXT_SUB, wd[i]);
        lv_obj_set_pos(l, 14 + i * 62, 12);
    }

    /* 日期格子 */
    int cell = first_wday;   /* 1 号落在第几格 */
    for(int d = 1; d <= days; d++, cell++) {
        int col = cell % 7;
        int row = cell / 7;

        lv_obj_t * l = ui_label(grid, UI_FONT_NORMAL, TEXT_MAIN, "");
        lv_label_set_text_fmt(l, "%d", d);
        lv_obj_set_pos(l, 22 + col * 62, 44 + row * 42);

        if(d == today) {
            lv_obj_t * dot = ui_pill(grid, 34, 34, lv_color_hex(0x2196F3), LV_OPA_COVER);
            lv_obj_set_pos(dot, 12 + col * 62, 38 + row * 42);
            lv_obj_move_background(dot);
            lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_pos(l, 22 + col * 62, 44 + row * 42);
            lv_obj_move_foreground(l);
        }
    }
    return page;
}

/* ===================== Health ===================== */

lv_obj_t * health_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Health", "today");

    lv_obj_t * c1 = card(page, lv_pct(88), 130, NULL, 120, CARD_BG);
    lv_obj_t * v1 = ui_label(c1, UI_FONT_HUGE, lv_color_hex(0x2FBF71), "7,842");
    lv_obj_align(v1, LV_ALIGN_LEFT_MID, 24, -10);
    lv_obj_t * l1 = ui_label(c1, UI_FONT_NORMAL, TEXT_SUB, "steps");
    lv_obj_align_to(l1, v1, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 2);

    lv_obj_t * c2 = card(page, lv_pct(88), 130, c1, 14, CARD_BG);
    lv_obj_t * v2 = ui_label(c2, UI_FONT_HUGE, lv_color_hex(0xF44336), "72");
    lv_obj_align(v2, LV_ALIGN_LEFT_MID, 24, -10);
    lv_obj_t * l2 = ui_label(c2, UI_FONT_NORMAL, TEXT_SUB, "bpm");
    lv_obj_align_to(l2, v2, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 2);

    return page;
}

/* ===================== Home（智能家居开关） ===================== */

static void switch_event_cb(lv_event_t * e)
{
    (void)e;
}

lv_obj_t * home_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Home", "smart devices");

    static const char * const names[] = { "Living room", "Bedroom", "Kitchen", "Balcony" };
    static const bool states[] = { true, false, true, false };

    lv_obj_t * prev = NULL;
    for(int i = 0; i < 4; i++) {
        lv_obj_t * row = card(page, lv_pct(88), 72, prev, prev ? 8 : 118, CARD_BG);
        prev = row;

        lv_obj_t * n = ui_label(row, UI_FONT_NORMAL, TEXT_MAIN, names[i]);
        lv_obj_align(n, LV_ALIGN_LEFT_MID, 20, 0);

        lv_obj_t * sw = lv_switch_create(row);
        lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -18, 0);
        lv_obj_set_size(sw, 56, 30);
        if(states[i]) lv_obj_add_state(sw, LV_STATE_CHECKED);
        lv_obj_add_event_cb(sw, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    }
    return page;
}

/* ===================== Music ===================== */

typedef struct {
    lv_obj_t *   bar;
    lv_obj_t *   time_label;
    lv_obj_t *   play_label;
    bool         playing;
    uint32_t     pos_ms;      /* 已播放毫秒（0 ~ 200000） */
    uint32_t     last_tick;
    lv_timer_t * timer;
} music_t;

#define MUSIC_LEN_MS 200000

static void music_refresh(music_t * m)
{
    uint32_t pct = (uint32_t)((uint64_t)m->pos_ms * 100 / MUSIC_LEN_MS);
    lv_bar_set_value(m->bar, (int32_t)pct, LV_ANIM_OFF);
    lv_label_set_text_fmt(m->time_label, "%u:%02u / 3:20",
                          (unsigned)(m->pos_ms / 60000u), (unsigned)((m->pos_ms / 1000u) % 60u));
}

static void music_timer_cb(lv_timer_t * t)
{
    music_t * m = (music_t *)t->user_data;
    if(m == NULL || !m->playing) return;

    uint32_t now = lv_tick_get();
    m->pos_ms += lv_tick_elaps(m->last_tick);
    m->last_tick = now;
    if(m->pos_ms >= MUSIC_LEN_MS) m->pos_ms = 0;
    music_refresh(m);
}

static void music_play_cb(lv_event_t * e)
{
    music_t * m = (music_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    m->playing = !m->playing;
    m->last_tick = lv_tick_get();
    lv_label_set_text(m->play_label, m->playing ? "Pause" : "Play");
}

static void music_page_deleted_cb(lv_event_t * e)
{
    music_t * m = (music_t *)lv_event_get_user_data(e);
    if(m == NULL) return;
    if(m->timer) lv_timer_del(m->timer);
    lv_mem_free(m);
}

lv_obj_t * music_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Music", "now playing");

    music_t * m = (music_t *)lv_mem_alloc(sizeof(music_t));
    lv_memset_00(m, sizeof(music_t));

    lv_obj_t * cover = card(page, 190, 190, NULL, 110, lv_color_hex(0x4C6FFF));
    lv_obj_set_style_radius(cover, 24, 0);
    lv_obj_t * note = ui_label(cover, UI_FONT_HUGE, lv_color_hex(0xFFFFFF), "LVGL");
    lv_obj_center(note);

    lv_obj_t * song = ui_label(page, UI_FONT_NORMAL, TEXT_MAIN, "Simulator Song");
    lv_obj_align(song, LV_ALIGN_TOP_MID, 0, 316);

    m->bar = lv_bar_create(page);
    lv_obj_set_size(m->bar, lv_pct(70), 10);
    lv_obj_align(m->bar, LV_ALIGN_TOP_MID, 0, 348);
    lv_bar_set_range(m->bar, 0, 100);
    lv_bar_set_value(m->bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(m->bar, lv_color_hex(0xD8D8DE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(m->bar, lv_color_hex(0x4C6FFF), LV_PART_INDICATOR);

    m->time_label = ui_label(page, UI_FONT_SMALL, TEXT_SUB, "0:00 / 3:20");
    lv_obj_align_to(m->time_label, m->bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    lv_obj_t * btn = lv_btn_create(page);
    lv_obj_set_size(btn, 140, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 386);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x4C6FFF), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, music_play_cb, LV_EVENT_CLICKED, m);

    m->play_label = ui_label(btn, UI_FONT_NORMAL, lv_color_hex(0xFFFFFF), "Play");
    lv_obj_center(m->play_label);

    m->timer = lv_timer_create(music_timer_cb, 200, m);
    lv_obj_add_event_cb(page, music_page_deleted_cb, LV_EVENT_DELETE, m);

    return page;
}

/* ===================== Notes ===================== */

lv_obj_t * notes_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Notes", "tap to edit");

    lv_obj_t * box = card(page, lv_pct(88), 260, NULL, 120, CARD_BG);

    lv_obj_t * ta = lv_textarea_create(box);
    lv_obj_set_size(ta, lv_pct(92), lv_pct(88));
    lv_obj_center(ta);
    lv_textarea_set_text(ta,
                         "LVGL 8 on PC\n\n"
                         "- lock screen: swipe up to unlock\n"
                         "- home screen: swipe left / right to page\n"
                         "- app: drag up from the bottom edge to close\n"
                         "- press 'L' to lock again\n");
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_style_border_width(ta, 0, 0);
    lv_obj_set_style_text_font(ta, UI_FONT_SMALL, 0);
    return page;
}

/* ===================== Weather ===================== */

lv_obj_t * weather_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;
    lv_obj_t * page = page_create(parent, "Shenzhen", "Partly cloudy");

    lv_obj_t * big = ui_label(page, UI_FONT_HUGE, TEXT_MAIN, "26°");
    lv_obj_align(big, LV_ALIGN_TOP_MID, 0, 130);

    static const char * const days[] = { "Mon", "Tue", "Wed" };
    static const char * const temps[] = { "27° / 21°", "25° / 20°", "24° / 19°" };
    static const uint32_t colors[] = { 0xFFC94D, 0x9AA0A6, 0x4C6FFF };

    lv_obj_t * prev = NULL;
    for(int i = 0; i < 3; i++) {
        lv_obj_t * col = card(page, 150, 120, NULL, 0, CARD_BG);
        if(prev == NULL) lv_obj_align(col, LV_ALIGN_TOP_LEFT, 42, 220);
        else             lv_obj_align_to(col, prev, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
        prev = col;

        lv_obj_t * dot = ui_pill(col, 36, 36, lv_color_hex(colors[i]), LV_OPA_COVER);
        lv_obj_align(dot, LV_ALIGN_TOP_MID, 0, 12);

        lv_obj_t * d = ui_label(col, UI_FONT_NORMAL, TEXT_MAIN, days[i]);
        lv_obj_align(d, LV_ALIGN_CENTER, 0, 6);

        lv_obj_t * tp = ui_label(col, UI_FONT_SMALL, TEXT_SUB, temps[i]);
        lv_obj_align(tp, LV_ALIGN_BOTTOM_MID, 0, -12);
    }
    return page;
}
