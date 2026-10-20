/**
 * @file status_bar.c
 */
#include "myCode/GUI/status_bar.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <stdio.h>

struct status_bar {
    lv_obj_t * cont;
    lv_obj_t * time_label;
};

static void y_anim_cb(void * var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, v);
}

static void animate_y(status_bar_t * bar, lv_coord_t from, lv_coord_t to)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar->cont);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_time(&a, UI_ANIM_UNLOCK);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, y_anim_cb);
    lv_anim_start(&a);
}

status_bar_t * status_bar_create(lv_obj_t * parent)
{
    status_bar_t * bar = (status_bar_t *)lv_mem_alloc(sizeof(status_bar_t));
    lv_memset_00(bar, sizeof(status_bar_t));

    bar->cont = ui_plain_obj(parent, lv_pct(100), UI_STATUS_BAR_H);
    lv_obj_align(bar->cont, LV_ALIGN_TOP_MID, 0, 0);
    /* lv_obj_create() 默认是 clickable 的。状态栏会压在 App 上面（非全屏时），
     * 清掉这个标志，让点击穿透到下面的 App 内容，而不是被状态栏吃掉。 */
    lv_obj_clear_flag(bar->cont, LV_OBJ_FLAG_CLICKABLE);

    bar->time_label = ui_label(bar->cont, UI_FONT_NORMAL, UI_COLOR_TEXT_DARK, "00:00");
    lv_obj_align(bar->time_label, LV_ALIGN_LEFT_MID, 20, 0);

    return bar;
}

void status_bar_set_time(status_bar_t * bar, int hour, int minute)
{
    if(bar == NULL) return;
    lv_label_set_text_fmt(bar->time_label, "%02d:%02d", hour, minute);
}

void status_bar_set_text_color(status_bar_t * bar, lv_color_t color)
{
    if(bar == NULL) return;
    lv_obj_set_style_text_color(bar->time_label, color, 0);
}

void status_bar_enter_full_screen(status_bar_t * bar)
{
    if(bar == NULL) return;
    animate_y(bar, 0, -UI_STATUS_BAR_H);
}

void status_bar_exit_full_screen(status_bar_t * bar)
{
    if(bar == NULL) return;
    animate_y(bar, -UI_STATUS_BAR_H, 0);
}

lv_obj_t * status_bar_obj(status_bar_t * bar)
{
    return bar ? bar->cont : NULL;
}
