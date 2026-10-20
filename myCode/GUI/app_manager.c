/**
 * @file app_manager.c
 * 应用图标与 App 打开/关闭动画。
 */
#include "myCode/GUI/app_manager.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"
#include "myCode/GUI/msg_box.h"

#include <string.h>

#define ICON_INDEX_MAX 32

struct app_manager {
    lv_obj_t *      screen;
    status_bar_t *  status_bar;
    void *          user_ctx;

    lv_obj_t *      masker;        /* 缩放动画的载体 */
    lv_obj_t *      app_conv;      /* 当前 App 的内容容器（masker 的子对象） */
    lv_obj_t *      app_home_bar;  /* App 内底部的黑色小条 */
    lv_obj_t *      app_drag_area; /* App 底部拖拽热区 */

    struct {
        const app_entry_t * entry;
        lv_obj_t *          icon;
    } icons[ICON_INDEX_MAX];
    int icon_cnt;

    const app_entry_t * current;       /* 当前打开的 App */
    lv_area_t           current_icon_area;
    bool                is_open;
    bool                full_screen;

    /* 拖拽过程中的临时状态（对应原代码的 bar_handle_start_x / bar_handle_prev_y） */
    lv_coord_t drag_start_x;
    lv_coord_t drag_prev_y;
    bool       dragging;
};

/* ============ 动画小工具 ============ */

static void anim_set_x(void * var, int32_t v) { lv_obj_set_x((lv_obj_t *)var, v); }
static void anim_set_y(void * var, int32_t v) { lv_obj_set_y((lv_obj_t *)var, v); }
static void anim_set_w(void * var, int32_t v) { lv_obj_set_width((lv_obj_t *)var, v); }
static void anim_set_h(void * var, int32_t v) { lv_obj_set_height((lv_obj_t *)var, v); }
static void anim_set_opa(void * var, int32_t v) { lv_obj_set_style_opa((lv_obj_t *)var, (lv_opa_t)v, 0); }

static void anim_start(lv_obj_t * obj, lv_anim_exec_xcb_t cb, int32_t from, int32_t to, uint32_t time_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_time(&a, time_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, cb);
    lv_anim_start(&a);
}

/* ============ 创建 ============ */

app_manager_t * app_manager_create(lv_obj_t * screen, status_bar_t * status_bar, void * user_ctx)
{
    app_manager_t * mgr = (app_manager_t *)lv_mem_alloc(sizeof(app_manager_t));
    lv_memset_00(mgr, sizeof(app_manager_t));
    mgr->screen     = screen;
    mgr->status_bar = status_bar;
    mgr->user_ctx   = user_ctx;
    return mgr;
}

/* ============ 图标：按下放大 / 点击打开 ============ */

static void icon_zoom_anim_cb(void * var, int32_t v)
{
    lv_img_set_zoom((lv_obj_t *)var, (uint16_t)v);
}

static void icon_zoom(lv_obj_t * icon, int32_t from, int32_t to)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, icon);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_time(&a, 100);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, icon_zoom_anim_cb);
    lv_anim_start(&a);
}

static void icon_event_cb(lv_event_t * e)
{
    lv_event_code_t code      = lv_event_get_code(e);
    lv_obj_t *      icon      = lv_event_get_target(e);
    const app_entry_t * entry = (const app_entry_t *)lv_obj_get_user_data(icon);
    app_manager_t * mgr       = (app_manager_t *)lv_event_get_user_data(e);
    if(entry == NULL || mgr == NULL) return;

    if(code == LV_EVENT_PRESSED) {
        icon_zoom(icon, 256, 270);
    }
    else if(code == LV_EVENT_RELEASED) {
        icon_zoom(icon, 270, 256);
    }
    else if(code == LV_EVENT_CLICKED) {
        lv_area_t area;
        lv_obj_get_coords(icon, &area);
        app_manager_open(mgr, entry, &area);
    }
}

void app_manager_make_icon(app_manager_t * mgr, lv_obj_t * slot, const app_entry_t * entry,
                           lv_color_t label_color)
{
    lv_obj_t * icon = ui_img(slot, entry->icon);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(icon, (void *)entry);

    lv_obj_add_event_cb(icon, icon_event_cb, LV_EVENT_PRESSED, mgr);
    lv_obj_add_event_cb(icon, icon_event_cb, LV_EVENT_RELEASED, mgr);
    lv_obj_add_event_cb(icon, icon_event_cb, LV_EVENT_CLICKED, mgr);

    lv_obj_t * label = ui_label(slot, UI_FONT_NORMAL, label_color, entry->name);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -8);

    if(mgr->icon_cnt < ICON_INDEX_MAX) {
        mgr->icons[mgr->icon_cnt].entry = entry;
        mgr->icons[mgr->icon_cnt].icon  = icon;
        mgr->icon_cnt++;
    }
}

/* ============ 在 App 里上滑关闭 ============ */

static void close_after_anim_timer_cb(lv_timer_t * t)
{
    app_manager_t * mgr = (app_manager_t *)t->user_data;
    app_manager_close(mgr);
}

/** 把 masker 从当前大小缩回图标位置，动画结束后真正关闭 App。
 *  拖拽松手、以及 app_manager_close_animated() 都走这里。 */
static void start_close_animation(app_manager_t * mgr)
{
    lv_coord_t mx = lv_obj_get_x(mgr->masker);
    lv_coord_t my = lv_obj_get_y(mgr->masker);
    lv_coord_t mw = lv_obj_get_width(mgr->masker);
    lv_coord_t mh = lv_obj_get_height(mgr->masker);

    lv_coord_t tw = mgr->current_icon_area.x2 - mgr->current_icon_area.x1 + 1;
    lv_coord_t th = mgr->current_icon_area.y2 - mgr->current_icon_area.y1 + 1;

    anim_start(mgr->masker, anim_set_w,   mw, tw, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_h,   mh, th, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_x,   mx, mgr->current_icon_area.x1, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_y,   my, mgr->current_icon_area.y1, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_opa, LV_OPA_COVER, LV_OPA_TRANSP, UI_APP_ANIM_TIME);

    /* 单次定时器：动画放完再收尾，避免在动画回调里删对象 */
    lv_timer_t * t = lv_timer_create(close_after_anim_timer_cb, UI_APP_ANIM_TIME + 20, mgr);
    lv_timer_set_repeat_count(t, 1);
}

static void app_drag_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    app_manager_t * mgr  = (app_manager_t *)lv_event_get_user_data(e);
    if(mgr == NULL || mgr->masker == NULL) return;

    lv_coord_t sw = lv_obj_get_width(mgr->screen);
    lv_coord_t sh = lv_obj_get_height(mgr->screen);

    if(code == LV_EVENT_PRESSING) {
        lv_point_t p;
        lv_indev_get_point(lv_indev_get_act(), &p);

        if(!mgr->dragging) {
            mgr->dragging     = true;
            mgr->drag_start_x = p.x;
            mgr->drag_prev_y  = p.y;
            return;
        }

        /* 手指越靠下，masker 缩得越小；最小不低于屏幕的 50% */
        int32_t size_pct = 1000 - (int32_t)(((int64_t)(sh - p.y) * 1000) / sh);
        if(size_pct < 500) size_pct = 500;

        lv_coord_t mw = (lv_coord_t)(((int64_t)sw * size_pct) / 1000);
        lv_coord_t mh = (lv_coord_t)(((int64_t)sh * size_pct) / 1000);
        lv_coord_t mx = (sw - mw) / 2 + p.x - mgr->drag_start_x;
        lv_coord_t my = p.y - mgr->drag_prev_y
                        + lv_obj_get_y(mgr->masker)
                        + lv_obj_get_height(mgr->masker) - mh;

        lv_obj_set_size(mgr->masker, mw, mh);
        lv_obj_set_pos(mgr->masker, mx, my);
        mgr->drag_prev_y = p.y;
    }
    else if(code == LV_EVENT_RELEASED) {
        if(!mgr->dragging) return;
        mgr->dragging = false;

        if(mgr->drag_prev_y <= (sh * 3) / 4) {
            /* 拖得够高：缩回图标位置并关闭 */
            start_close_animation(mgr);
        }
        else {
            /* 没拖够：弹回全屏 */
            lv_coord_t mx = lv_obj_get_x(mgr->masker);
            lv_coord_t my = lv_obj_get_y(mgr->masker);
            lv_coord_t mw = lv_obj_get_width(mgr->masker);
            lv_coord_t mh = lv_obj_get_height(mgr->masker);

            anim_start(mgr->masker, anim_set_w,   mw, sw, 100);
            anim_start(mgr->masker, anim_set_h,   mh, sh, 100);
            anim_start(mgr->masker, anim_set_x,   mx, 1, 100);
            anim_start(mgr->masker, anim_set_y,   my, 1, 100);
            anim_start(mgr->masker, anim_set_opa,
                       lv_obj_get_style_opa(mgr->masker, 0), LV_OPA_COVER, 100);
        }
    }
}

/* ============ App 打开 / 关闭 ============ */

static void masker_create(app_manager_t * mgr)
{
    lv_coord_t sw = lv_obj_get_width(mgr->screen);
    lv_coord_t sh = lv_obj_get_height(mgr->screen);

    mgr->masker = lv_obj_create(mgr->screen);
    lv_obj_set_style_pad_all(mgr->masker, 0, 0);
    lv_obj_set_style_border_width(mgr->masker, 0, 0);
    lv_obj_set_style_radius(mgr->masker, UI_SCREEN_RADIUS, 0);
    lv_obj_set_style_clip_corner(mgr->masker, true, 0);
    lv_obj_set_style_bg_color(mgr->masker, UI_COLOR_MASK, 0);
    lv_obj_set_style_bg_opa(mgr->masker, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(mgr->masker, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(mgr->masker, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(mgr->masker, LV_OBJ_FLAG_HIDDEN);

    /* App 内容容器：和屏幕一样大，底部对齐。
     * 这样 masker 从上方收缩时，App 的顶部先被裁掉，像"从底部收起来"。 */
    mgr->app_conv = ui_plain_obj(mgr->masker, sw, sh);
    lv_obj_align(mgr->app_conv, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(mgr->app_conv, UI_COLOR_BG_TOP, 0);
    lv_obj_set_style_bg_opa(mgr->app_conv, LV_OPA_COVER, 0);

    /* App 内的 home bar（黑色小条） */
    mgr->app_home_bar = ui_pill(mgr->app_conv, lv_pct(50), 12, UI_COLOR_TEXT_DARK, LV_OPA_COVER);
    lv_obj_align(mgr->app_home_bar, LV_ALIGN_BOTTOM_MID, 0, -10);

    /* 底部拖拽热区：透明、可点击，专门收 PRESSING / RELEASED。
     * 原代码是 50% x 22（只在中下方一条窄带），这里放宽到整条底边 x 40px，
     * 从屏幕底边任意位置往上拖都能关 App，更好点中。 */
    mgr->app_drag_area = ui_plain_obj(mgr->app_conv, lv_pct(100), UI_APP_DRAG_H);
    lv_obj_align(mgr->app_drag_area, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(mgr->app_drag_area, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(mgr->app_drag_area, app_drag_event_cb, LV_EVENT_PRESSING, mgr);
    lv_obj_add_event_cb(mgr->app_drag_area, app_drag_event_cb, LV_EVENT_RELEASED, mgr);
}

void app_manager_open(app_manager_t * mgr, const app_entry_t * entry, const lv_area_t * icon_area)
{
    if(mgr == NULL || entry == NULL || mgr->is_open) return;

    /* 没有实现内容的 App 只弹提示框，不进入缩放动画流程 */
    if(entry->create == NULL) {
        msg_box_show(mgr->screen, "Message", "The app has not been implemented yet", "OK");
        return;
    }

    lv_coord_t sw = lv_obj_get_width(mgr->screen);
    lv_coord_t sh = lv_obj_get_height(mgr->screen);

    mgr->current           = entry;
    mgr->current_icon_area = *icon_area;
    mgr->is_open           = true;
    mgr->full_screen       = entry->full_screen;

    if(mgr->masker == NULL) masker_create(mgr);
    else {
        /* 清掉上一次 App 留下的内容 */
        uint32_t n = lv_obj_get_child_cnt(mgr->app_conv);
        for(int i = (int)n - 1; i >= 0; i--) {
            lv_obj_t * child = lv_obj_get_child(mgr->app_conv, (uint32_t)i);
            if(child == mgr->app_home_bar || child == mgr->app_drag_area) continue;
            lv_obj_del(child);
        }
    }

    lv_obj_align(mgr->app_home_bar, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_align(mgr->app_drag_area, LV_ALIGN_BOTTOM_MID, 0, 0);

    /* 让 App 自己往里画界面。第二个参数就是 app_manager_t*，
     * App 想全屏 / 想关掉自己时可以用它（对应原代码里传进去的 status_bar 和 close_cb）。 */
    entry->create(mgr->app_conv, mgr);

    /* ⚠️ 顺序很重要：App 内容的根页面是满屏的，而且 lv_obj_create() 默认带
     * LV_OBJ_FLAG_CLICKABLE —— 它会盖住底部拖拽热区，导致"上滑关不掉 App"。
     * 所以必须在 App 建完界面之后再抬一次 home bar / 拖拽热区。
     * 原 main.py 的 AppBase.set_home_bar_top() 也是在子类建完界面之后才调用的。 */
    lv_obj_move_foreground(mgr->app_home_bar);
    lv_obj_move_foreground(mgr->app_drag_area);

    if(entry->full_screen) {
        /* 全屏 App：状态栏滑出屏幕，home bar 下移 22px */
        status_bar_enter_full_screen(mgr->status_bar);
        anim_start(mgr->app_home_bar, anim_set_y, -10, 12, UI_ANIM_UNLOCK);
    }
    /* 深色背景的 App 用白字，浅色背景用黑字（对应原代码的 status_bar.dark_mode() / light_mode()） */
    status_bar_set_text_color(mgr->status_bar,
                              entry->dark_bg ? UI_COLOR_TEXT_LIGHT : UI_COLOR_TEXT_DARK);

    /* masker 从图标所在的位置"长"到全屏 */
    lv_coord_t cx = (icon_area->x1 + icon_area->x2) / 2;
    lv_coord_t cy = (icon_area->y1 + icon_area->y2) / 2;

    lv_obj_set_pos(mgr->masker, cx, cy);
    lv_obj_set_size(mgr->masker, 1, 1);
    lv_obj_set_style_opa(mgr->masker, LV_OPA_COVER, 0);
    lv_obj_clear_flag(mgr->masker, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(mgr->masker);

    /* 非全屏 App：masker 抬到最上层之后，再把状态栏压回去，
     * 这样顶部时间一直可见（全屏 App 就让状态栏留在下面）。 */
    if(!entry->full_screen) {
        lv_obj_t * sb = status_bar_obj(mgr->status_bar);
        if(sb) lv_obj_move_foreground(sb);
    }

    anim_start(mgr->masker, anim_set_x, cx, 1, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_y, cy, 1, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_w, 1, sw, UI_APP_ANIM_TIME);
    anim_start(mgr->masker, anim_set_h, 1, sh, UI_APP_ANIM_TIME);
}

void app_manager_close(app_manager_t * mgr)
{
    if(mgr == NULL || !mgr->is_open || mgr->masker == NULL) return;

    bool was_full_screen = mgr->full_screen;
    mgr->is_open     = false;
    mgr->current     = NULL;
    mgr->full_screen = false;

    /* 删掉 App 创建的内容，保留 app_conv / home bar / 拖拽区这几个骨架 */
    uint32_t n = lv_obj_get_child_cnt(mgr->app_conv);
    for(int i = (int)n - 1; i >= 0; i--) {
        lv_obj_t * child = lv_obj_get_child(mgr->app_conv, (uint32_t)i);
        if(child == mgr->app_home_bar || child == mgr->app_drag_area) continue;
        lv_obj_del(child);
    }

    lv_obj_add_flag(mgr->masker, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_opa(mgr->masker, LV_OPA_COVER, 0);
    lv_obj_set_size(mgr->masker, lv_obj_get_width(mgr->screen), lv_obj_get_height(mgr->screen));

    status_bar_set_text_color(mgr->status_bar, UI_COLOR_TEXT_DARK);
    if(was_full_screen) status_bar_exit_full_screen(mgr->status_bar);
}

bool app_manager_is_open(app_manager_t * mgr)
{
    return mgr && mgr->is_open;
}

void app_manager_close_animated(app_manager_t * mgr)
{
    if(mgr == NULL || !mgr->is_open || mgr->masker == NULL) return;
    start_close_animation(mgr);
}

void app_manager_set_full_screen(app_manager_t * mgr, bool full_screen)
{
    if(mgr == NULL || mgr->masker == NULL) return;
    if(mgr->full_screen == full_screen) return;

    mgr->full_screen = full_screen;
    if(full_screen) {
        status_bar_enter_full_screen(mgr->status_bar);
        anim_start(mgr->app_home_bar, anim_set_y, -10, 12, UI_ANIM_UNLOCK);
    }
    else {
        status_bar_exit_full_screen(mgr->status_bar);
        anim_start(mgr->app_home_bar, anim_set_y, 12, -10, UI_ANIM_UNLOCK);
    }
}

bool app_manager_open_by_name(app_manager_t * mgr, const char * name)
{
    if(mgr == NULL || name == NULL) return false;
    for(int i = 0; i < mgr->icon_cnt; i++) {
        if(strcmp(mgr->icons[i].entry->name, name) == 0) {
            lv_area_t area;
            lv_obj_get_coords(mgr->icons[i].icon, &area);
            app_manager_open(mgr, mgr->icons[i].entry, &area);
            return true;
        }
    }
    return false;
}
