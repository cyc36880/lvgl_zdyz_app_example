/**
 * @file settings_app.c
 *
 * 原 main.py 的 AppSettings 结构：
 *   AppSettings
 *     └── SideBar（左栏）
 *           ├── Profile：头像 + "正点原子" + "ALIENTEK"
 *           └── System：列表项 "Date & Time" —— 点击后在右栏生成设置界面
 *
 * 右栏那份设置界面是两组 lv.roller（年/月/日 + 时/分/秒），
 * 值一改就写回 ClockManager（硬件 RTC）。PC 上写回的是 Common/clock_model.c
 * 那个"真实时间 + 偏移"的软时钟。
 */
#include "myCode/APP/Settings/settings_app.h"
#include "myCode/Common/assets.h"
#include "myCode/Common/clock_model.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <stdio.h>

#define YEAR_FROM 2010
#define YEAR_TO   2039

typedef struct {
    lv_obj_t * item_conv;      /* 右栏容器 */
    lv_obj_t * handle;         /* "Date & Time" 的点击热区 */

    lv_obj_t * year;
    lv_obj_t * month;
    lv_obj_t * day;
    lv_obj_t * hour;
    lv_obj_t * minute;
    lv_obj_t * second;

    bool       building;       /* 正在程序化设置滚动值，忽略 VALUE_CHANGED */
} settings_t;

/* ---------------- 滚轮 ---------------- */

static void make_options(char * buf, size_t size, int from, int to)
{
    size_t off = 0;
    buf[0] = '\0';
    for(int v = from; v <= to && off + 8 < size; v++) {
        off += (size_t)snprintf(buf + off, size - off, "%d\n", v);
    }
}

static lv_obj_t * make_roller(lv_obj_t * parent, lv_obj_t * align_to, const char * options,
                              lv_obj_t * label_target, const char * label_text)
{
    lv_obj_t * roller = lv_roller_create(parent);
    lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
    lv_obj_set_style_text_font(roller, UI_FONT_NORMAL, 0);
    lv_obj_set_width(roller, 92);
    lv_roller_set_visible_row_count(roller, 3);

    if(align_to == NULL) {
        lv_obj_align(roller, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    }
    else {
        lv_obj_align_to(roller, align_to, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    }

    if(label_text) {
        lv_obj_t * label = ui_label(parent, UI_FONT_SMALL, lv_color_hex(0x7F7F7F), label_text);
        lv_obj_align_to(label, roller, LV_ALIGN_OUT_TOP_MID, 0, -4);
        (void)label_target;
    }
    return roller;
}

static void settings_apply(settings_t * s)
{
    if(s->building) return;
    if(s->year == NULL) return;

    int year   = (int)lv_roller_get_selected(s->year) + YEAR_FROM;
    int month  = (int)lv_roller_get_selected(s->month) + 1;
    int day    = (int)lv_roller_get_selected(s->day) + 1;
    int hour   = (int)lv_roller_get_selected(s->hour);
    int minute = (int)lv_roller_get_selected(s->minute);
    int second = (int)lv_roller_get_selected(s->second);

    clock_model_set(year, month, day, hour, minute, second);
}

static void roller_event_cb(lv_event_t * e)
{
    settings_t * s = (settings_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        settings_apply(s);
    }
}

/* ---------------- 右栏：日期 / 时间 设置 ---------------- */

static void build_item_conv(settings_t * s)
{
    char options[2048];

    /* 清空右栏 */
    lv_obj_clean(s->item_conv);
    s->year = s->month = s->day = s->hour = s->minute = s->second = NULL;

    /* ---- 日期卡片 ---- */
    lv_obj_t * date_card = lv_obj_create(s->item_conv);
    lv_obj_set_size(date_card, lv_pct(90), 200);
    lv_obj_align(date_card, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_pad_all(date_card, 0, 0);
    lv_obj_set_style_border_width(date_card, 0, 0);
    lv_obj_set_style_radius(date_card, 12, 0);
    lv_obj_set_style_clip_corner(date_card, true, 0);
    lv_obj_set_style_bg_color(date_card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(date_card, LV_OPA_COVER, 0);
    lv_obj_clear_flag(date_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * date_title = ui_label(date_card, UI_FONT_NORMAL, lv_color_hex(0x000000), "Date Settings");
    lv_obj_align(date_title, LV_ALIGN_TOP_LEFT, 10, 10);

    make_options(options, sizeof(options), YEAR_FROM, YEAR_TO);
    s->year = make_roller(date_card, NULL, options, NULL, "Year");
    make_options(options, sizeof(options), 1, 12);
    s->month = make_roller(date_card, s->year, options, NULL, "Month");
    make_options(options, sizeof(options), 1, 31);
    s->day = make_roller(date_card, s->month, options, NULL, "Date");

    /* ---- 时间卡片 ---- */
    lv_obj_t * time_card = lv_obj_create(s->item_conv);
    lv_obj_set_size(time_card, lv_pct(90), 200);
    lv_obj_align_to(time_card, date_card, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_pad_all(time_card, 0, 0);
    lv_obj_set_style_border_width(time_card, 0, 0);
    lv_obj_set_style_radius(time_card, 12, 0);
    lv_obj_set_style_clip_corner(time_card, true, 0);
    lv_obj_set_style_bg_color(time_card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(time_card, LV_OPA_COVER, 0);
    lv_obj_clear_flag(time_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * time_title = ui_label(time_card, UI_FONT_NORMAL, lv_color_hex(0x000000), "Time Settings");
    lv_obj_align(time_title, LV_ALIGN_TOP_LEFT, 10, 10);

    make_options(options, sizeof(options), 0, 23);
    s->hour = make_roller(time_card, NULL, options, NULL, "Hour");
    make_options(options, sizeof(options), 0, 59);
    s->minute = make_roller(time_card, s->hour, options, NULL, "Minute");
    s->second = make_roller(time_card, s->minute, options, NULL, "Second");

    /* ---- 用当前时间把滚轮摆好 ---- */
    struct tm tmv;
    clock_model_get(&tmv);

    s->building = true;
    int y = tmv.tm_year + 1900;
    if(y < YEAR_FROM) y = YEAR_FROM;
    if(y > YEAR_TO)   y = YEAR_TO;
    lv_roller_set_selected(s->year,   (uint16_t)(y - YEAR_FROM), LV_ANIM_OFF);
    lv_roller_set_selected(s->month,  (uint16_t)(tmv.tm_mon),    LV_ANIM_OFF);
    lv_roller_set_selected(s->day,    (uint16_t)(tmv.tm_mday - 1), LV_ANIM_OFF);
    lv_roller_set_selected(s->hour,   (uint16_t)tmv.tm_hour,     LV_ANIM_OFF);
    lv_roller_set_selected(s->minute, (uint16_t)tmv.tm_min,      LV_ANIM_OFF);
    lv_roller_set_selected(s->second, (uint16_t)tmv.tm_sec,      LV_ANIM_OFF);
    s->building = false;

    lv_obj_add_event_cb(s->year,   roller_event_cb, LV_EVENT_VALUE_CHANGED, s);
    lv_obj_add_event_cb(s->month,  roller_event_cb, LV_EVENT_VALUE_CHANGED, s);
    lv_obj_add_event_cb(s->day,    roller_event_cb, LV_EVENT_VALUE_CHANGED, s);
    lv_obj_add_event_cb(s->hour,   roller_event_cb, LV_EVENT_VALUE_CHANGED, s);
    lv_obj_add_event_cb(s->minute, roller_event_cb, LV_EVENT_VALUE_CHANGED, s);
    lv_obj_add_event_cb(s->second, roller_event_cb, LV_EVENT_VALUE_CHANGED, s);
}

static void item_clicked_cb(lv_event_t * e)
{
    settings_t * s = (settings_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    lv_obj_set_style_bg_opa(s->handle, 50, 0);      /* 点一下留个浅灰底，表示已选中 */
    build_item_conv(s);
}

/* ---------------- 创建 ---------------- */

lv_obj_t * settings_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;

    settings_t * s = (settings_t *)lv_mem_alloc(sizeof(settings_t));
    lv_memset_00(s, sizeof(settings_t));

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0xF3F2F7), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    /* ---- 左栏 ---- */
    lv_obj_t * side_bar = ui_plain_obj(page, lv_pct(44), lv_pct(100));
    lv_obj_align(side_bar, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * title = ui_label(side_bar, UI_FONT_TITLE, lv_color_hex(0x000000), "Settings");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 50);

    lv_obj_t * scroll = ui_plain_obj(side_bar, lv_pct(90), 380);
    lv_obj_align(scroll, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_add_flag(scroll, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll, LV_SCROLLBAR_MODE_OFF);

    /* 头像卡 */
    lv_obj_t * profile = lv_obj_create(scroll);
    lv_obj_set_size(profile, lv_pct(100), 92);
    lv_obj_align(profile, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_all(profile, 0, 0);
    lv_obj_set_style_border_width(profile, 0, 0);
    lv_obj_set_style_radius(profile, 12, 0);
    lv_obj_set_style_clip_corner(profile, true, 0);
    lv_obj_set_style_bg_color(profile, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(profile, LV_OPA_COVER, 0);
    lv_obj_clear_flag(profile, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * avatar = ui_img(profile, "APP/Settings/settings_icon_72x72_profile.png");
    lv_obj_align(avatar, LV_ALIGN_LEFT_MID, 10, 0);

    /* 原代码这里是 "正点原子"，需要一个中文字体；
     * lv_conf.h 里 LV_FONT_SIMSUN_16_CJK 是开着的，正好用上。 */
    lv_obj_t * user = ui_label(profile, &lv_font_simsun_16_cjk, lv_color_hex(0x000000), "正点原子");
    lv_obj_align_to(user, avatar, LV_ALIGN_OUT_RIGHT_MID, 16, -12);

    lv_obj_t * commit = ui_label(profile, UI_FONT_SMALL, lv_color_hex(0x7F7F7F), "ALIENTEK");
    lv_obj_align_to(commit, user, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    /* 系统项：Date & Time */
    lv_obj_t * system = lv_obj_create(scroll);
    lv_obj_set_size(system, lv_pct(100), 64);
    lv_obj_align_to(system, profile, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_pad_all(system, 0, 0);
    lv_obj_set_style_border_width(system, 0, 0);
    lv_obj_set_style_radius(system, 12, 0);
    lv_obj_set_style_clip_corner(system, true, 0);
    lv_obj_set_style_bg_color(system, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(system, LV_OPA_COVER, 0);
    lv_obj_clear_flag(system, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * date_icon = ui_img(system, "APP/Settings/settings_icon_44x44_date_and_time.png");
    lv_obj_align(date_icon, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_radius(date_icon, 12, 0);
    lv_obj_set_style_clip_corner(date_icon, true, 0);

    lv_obj_t * date_label = ui_label(system, UI_FONT_NORMAL, lv_color_hex(0x000000), "Date & Time");
    lv_obj_align_to(date_label, date_icon, LV_ALIGN_OUT_RIGHT_MID, 12, 0);

    s->handle = ui_plain_obj(system, lv_pct(100), lv_pct(100));
    lv_obj_center(s->handle);
    lv_obj_set_style_bg_color(s->handle, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s->handle, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(s->handle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(s->handle, item_clicked_cb, LV_EVENT_CLICKED, s);

    /* ---- 中间分隔线 ---- */
    lv_obj_t * split = ui_plain_obj(page, 2, lv_pct(100));
    lv_obj_align_to(split, side_bar, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(split, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(split, 50, 0);

    /* ---- 右栏 ---- */
    s->item_conv = ui_plain_obj(page, lv_pct(55), lv_pct(100));
    lv_obj_align(s->item_conv, LV_ALIGN_RIGHT_MID, 0, 0);

    /* 默认就把日期时间设置打开，免得用户以为右边是空的 */
    build_item_conv(s);

    return page;
}
