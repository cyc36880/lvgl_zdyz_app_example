/**
 * @file msg_box.c
 */
#include "myCode/GUI/msg_box.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

static lv_obj_t * s_masker = NULL;

void msg_box_close(void)
{
    if(s_masker) {
        lv_obj_del(s_masker);
        s_masker = NULL;
    }
}

static void ok_event_cb(lv_event_t * e)
{
    (void)e;
    msg_box_close();
}

void msg_box_show(lv_obj_t * parent, const char * title, const char * msg, const char * ok_text)
{
    msg_box_close();

    s_masker = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(s_masker, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_radius(s_masker, UI_SCREEN_RADIUS, 0);
    lv_obj_set_style_clip_corner(s_masker, true, 0);
    lv_obj_set_style_bg_color(s_masker, UI_COLOR_MASK, 0);
    lv_obj_set_style_bg_opa(s_masker, LV_OPA_50, 0);
    lv_obj_move_foreground(s_masker);

    lv_obj_t * conv = lv_obj_create(s_masker);
    lv_obj_set_size(conv, lv_pct(60), lv_pct(62));
    lv_obj_center(conv);
    lv_obj_set_style_pad_all(conv, 0, 0);
    lv_obj_set_style_border_width(conv, 0, 0);
    lv_obj_set_style_radius(conv, 20, 0);
    lv_obj_set_style_clip_corner(conv, true, 0);
    lv_obj_set_style_bg_color(conv, UI_COLOR_BG_TOP, 0);
    lv_obj_clear_flag(conv, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title_label = ui_label(conv, UI_FONT_NORMAL, UI_COLOR_TEXT_DARK, title ? title : "");
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 24, 20);

    lv_obj_t * msg_label = ui_label(conv, UI_FONT_SMALL, UI_COLOR_TEXT_DARK, msg ? msg : "");
    lv_obj_set_width(msg_label, lv_pct(80));
    lv_label_set_long_mode(msg_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(msg_label, LV_ALIGN_TOP_LEFT, 24, 54);

    lv_obj_t * btn = lv_btn_create(conv);
    lv_obj_set_size(btn, lv_pct(40), 46);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, UI_COLOR_ACCENT, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, ok_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_label = ui_label(btn, UI_FONT_NORMAL, UI_COLOR_TEXT_LIGHT, ok_text ? ok_text : "OK");
    lv_obj_center(btn_label);
}
