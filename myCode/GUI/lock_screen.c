/**
 * @file lock_screen.c
 */
#include "myCode/GUI/lock_screen.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"
#include "myCode/GUI/big_clock.h"

#define UNLOCK_OFFSCREEN_Y (-(UI_H + 40))

struct lock_screen {
    lv_obj_t *    conv;        /* 整屏容器，上下拖动就是移动它 */
    lv_obj_t *    hint;        /* "Swipe up to open" */
    lv_obj_t *    bar;         /* 底部白色小条 */
    lv_obj_t *    handle;      /* 底部透明拖拽热区 */
    big_clock_t * clock;

    bool visible;
    bool dragging;
    lv_coord_t drag_start_y;

    void (*on_unlock)(void * ctx);
    void *      ctx;
};

/* ---------- 动画 ---------- */

static void anim_set_y(void * var, int32_t v) { lv_obj_set_y((lv_obj_t *)var, v); }
static void anim_set_translate_y(void * var, int32_t v) { lv_obj_set_style_translate_y((lv_obj_t *)var, v, 0); }
static void anim_set_opa(void * var, int32_t v) { lv_obj_set_style_opa((lv_obj_t *)var, (lv_opa_t)v, 0); }

/** 一条"呼吸"动画：正放 + 倒放 + 延时循环（对应原代码那堆 set_playback_* / set_repeat_*）。 */
static void breathing_anim(lv_obj_t * obj, lv_anim_exec_xcb_t cb,
                           int32_t from, int32_t to, uint32_t time_ms,
                           uint32_t playback_delay, uint32_t playback_time,
                           uint32_t repeat_delay)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_time(&a, time_ms);
    lv_anim_set_playback_delay(&a, playback_delay);
    lv_anim_set_playback_time(&a, playback_time);
    lv_anim_set_repeat_delay(&a, repeat_delay);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, cb);
    lv_anim_start(&a);
}

/* ---------- 解锁动画收尾 ---------- */

static void unlock_ready_cb(lv_anim_t * a)
{
    lock_screen_t * ls = (lock_screen_t *)lv_anim_get_user_data(a);
    if(ls == NULL) return;
    ls->visible = false;
    lv_obj_add_flag(ls->conv, LV_OBJ_FLAG_HIDDEN);
    if(ls->on_unlock) ls->on_unlock(ls->ctx);
}

static void animate_conv_to(lock_screen_t * ls, lv_coord_t to, void (*ready)(lv_anim_t *))
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ls->conv);
    lv_anim_set_values(&a, lv_obj_get_y(ls->conv), to);
    lv_anim_set_time(&a, UI_ANIM_UNLOCK);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, anim_set_y);
    if(ready) {
        lv_anim_set_ready_cb(&a, ready);
        lv_anim_set_user_data(&a, ls);
    }
    lv_anim_start(&a);
}

/* ---------- 上滑手势 ---------- */

static void handle_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lock_screen_t * ls   = (lock_screen_t *)lv_event_get_user_data(e);
    if(ls == NULL || !ls->visible) return;

    if(code == LV_EVENT_PRESSING) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_get_act(), &p);

        if(!ls->dragging) {
            ls->dragging     = true;
            ls->drag_start_y = p.y;
            /* 手指按住时先停掉正在进行的回弹动画 */
            lv_anim_del(ls->conv, anim_set_y);
            return;
        }

        lv_coord_t delta = p.y - ls->drag_start_y;
        lv_coord_t dest  = lv_obj_get_y(ls->conv) + delta;
        if(dest <= 0) {
            lv_obj_set_y(ls->conv, dest);
        }
        ls->drag_start_y = p.y;
    }
    else if(code == LV_EVENT_RELEASED) {
        if(!ls->dragging) return;
        ls->dragging = false;

        /* 滑动超过屏幕高度的 1/3 就解锁，否则回弹（原代码同样的判断） */
        if(lv_obj_get_y(ls->conv) < -(UI_H / 3)) {
            animate_conv_to(ls, UNLOCK_OFFSCREEN_Y, unlock_ready_cb);
        }
        else {
            animate_conv_to(ls, 0, NULL);
        }
    }
}

/* ---------- 创建 ---------- */

lock_screen_t * lock_screen_create(lv_obj_t * parent,
                                   void (*on_unlock)(void * ctx), void * ctx)
{
    lock_screen_t * ls = (lock_screen_t *)lv_mem_alloc(sizeof(lock_screen_t));
    lv_memset_00(ls, sizeof(lock_screen_t));
    ls->on_unlock = on_unlock;
    ls->ctx       = ctx;

    /* 整屏渐变（上白下黑） */
    ls->conv = ui_gradient_bg(parent, lv_pct(100), lv_pct(100),
                              UI_COLOR_BG_TOP, UI_COLOR_BG_BOTTOM, 0);
    lv_obj_align(ls->conv, LV_ALIGN_TOP_LEFT, 0, 0);

    /* 大时钟：上方偏白的位置放黑字 */
    ls->clock = big_clock_create(ls->conv);
    big_clock_set_color(ls->clock, UI_COLOR_TEXT_DARK);
    lv_obj_align(big_clock_obj(ls->clock), LV_ALIGN_TOP_MID, 0, 90);

    /* 底部提示文字（下白上黑的背景里用白字） */
    ls->hint = ui_label(ls->conv, UI_FONT_NORMAL, UI_COLOR_TEXT_LIGHT, "Swipe up to open");
    lv_obj_align(ls->hint, LV_ALIGN_BOTTOM_MID, 0, -46);
    lv_obj_set_style_opa(ls->hint, LV_OPA_TRANSP, 0);

    /* 底部白色小条 */
    ls->bar = ui_pill(ls->conv, lv_pct(50), 10, UI_COLOR_TEXT_LIGHT, LV_OPA_COVER);
    lv_obj_align(ls->bar, LV_ALIGN_BOTTOM_MID, 0, -16);

    /* 拖拽热区：盖住底部 120px，透明但可交互 */
    ls->handle = ui_plain_obj(ls->conv, lv_pct(100), 120);
    lv_obj_align(ls->handle, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(ls->handle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ls->handle, handle_event_cb, LV_EVENT_PRESSING, ls);
    lv_obj_add_event_cb(ls->handle, handle_event_cb, LV_EVENT_RELEASED, ls);

    /* 三条"呼吸"动画：
     *   小条上下浮 15px、提示文字上浮并淡入淡出、两者节奏错开一点 */
    breathing_anim(ls->bar,  anim_set_translate_y, 0, -15, 1000, 500, 800, 2000);
    breathing_anim(ls->hint, anim_set_translate_y, 0, -10, 1000, 500, 800, 2000);
    breathing_anim(ls->hint, anim_set_opa,         LV_OPA_TRANSP, LV_OPA_COVER, 1000, 500, 800, 2000);

    ls->visible = true;
    return ls;
}

void lock_screen_show(lock_screen_t * ls)
{
    if(ls == NULL) return;

    lv_obj_set_y(ls->conv, -UI_H);
    lv_obj_clear_flag(ls->conv, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(ls->conv);
    ls->visible = true;
    animate_conv_to(ls, 0, NULL);
}

void lock_screen_hide_now(lock_screen_t * ls)
{
    if(ls == NULL) return;
    lv_obj_add_flag(ls->conv, LV_OBJ_FLAG_HIDDEN);
    ls->visible = false;
}

void lock_screen_set_time(lock_screen_t * ls, int hour, int minute)
{
    if(ls == NULL) return;
    big_clock_set_time(ls->clock, hour, minute);
}

bool lock_screen_is_visible(lock_screen_t * ls)
{
    return ls && ls->visible;
}

void lock_screen_force_unlock(lock_screen_t * ls)
{
    if(ls == NULL || !ls->visible) return;
    lv_anim_del(ls->conv, anim_set_y);
    animate_conv_to(ls, UNLOCK_OFFSCREEN_Y, unlock_ready_cb);
}
