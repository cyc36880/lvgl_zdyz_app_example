/**
 * @file photos_app.c
 *
 * 原 main.py 的 AppPhotos 有一个挺巧的做法：只建 3 个"整屏容器"，
 * 分别摆在 屏幕中间 / 上方 / 下方，里面各放一张 lv_img；
 * 上下滑动 + SCROLL_SNAP.CENTER 让它们像滚筒一样换位，
 * 在 SCROLL_END 时把索引 ±1、重新设置三张图、再把滚动位置归零 —— 于是"无限翻页"。
 *
 * 这里沿用这个思路，但把"轮换容器指针"简化成"三个固定槽位 + 每次重设 src"，
 * 视觉效果一样，代码更好读。
 */
#include "myCode/APP/Photos/photos_app.h"
#include "myCode/Common/assets.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/app_manager.h"
#include "myCode/GUI/widgets.h"

/* CanMV 那边是 os.listdir(Photos/) 动态读的；PC 上写死列表，省掉目录遍历。
 * 这里按"从大到小"排，打开第一眼就是铺满全屏的 640x480，
 * 往上滑依次是越来越小的图（原版是按目录顺序，第一张只有 136x102，显得很空）。 */
static const char * const s_photos[] = {
    "Photos/640x480_pexels-chevanon-1108099.png",
    "Photos/556x417_pexels-francesco-ungaro-2835436.png",
    "Photos/472x354_pexels-gochrisgoxyz-1477166.png",
    "Photos/388x291_pexels-madsdonald-2739013.png",
    "Photos/304x228_pexels-matthardy-3560168.png",
    "Photos/220x165_pexels-stywo-1054218.png",
    "Photos/136x102_pexels-stywo-1261728.png",
};

#define PHOTO_NUM ((int)(sizeof(s_photos) / sizeof(s_photos[0])))

typedef struct {
    lv_obj_t *  img_conv;      /* 可上下滑动的容器 */
    lv_obj_t *  slot[3];       /* 0 = 中间(当前)，1 = 上方(上一张)，2 = 下方(下一张) */
    lv_obj_t *  img[3];
    int         index;
    bool        ignore_next_scroll_end;
    bool        full_screen;
    lv_point_t  press_pt;
    void *      app_mgr;
} photos_t;

static void slot_set_image(photos_t * p, int slot, int index)
{
    if(index < 0) index = 0;
    if(index >= PHOTO_NUM) index = PHOTO_NUM - 1;

    const lv_img_dsc_t * dsc = assets_png(s_photos[index]);
    if(dsc) lv_img_set_src(p->img[slot], dsc);
    lv_obj_align(p->img[slot], LV_ALIGN_CENTER, -1, -1);
}

static void photos_reload(photos_t * p)
{
    slot_set_image(p, 0, p->index);                     /* 当前 */
    slot_set_image(p, 1, p->index > 0 ? p->index - 1 : p->index);            /* 上一张 */
    slot_set_image(p, 2, p->index < PHOTO_NUM - 1 ? p->index + 1 : p->index); /* 下一张 */
}

static void img_conv_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    photos_t * p = (photos_t *)lv_event_get_user_data(e);

    if(code != LV_EVENT_SCROLL_END) return;

    if(p->ignore_next_scroll_end) {
        p->ignore_next_scroll_end = false;
        return;
    }

    lv_coord_t sy = lv_obj_get_scroll_y(p->img_conv);
    if(sy == 0) return;

    if(sy > 0 && p->index < PHOTO_NUM - 1) {
        p->index++;                     /* 往上滑 → 看下一张 */
    }
    else if(sy < 0 && p->index > 0) {
        p->index--;                     /* 往下滑 → 看上一张 */
    }

    photos_reload(p);

    p->ignore_next_scroll_end = true;
    lv_obj_scroll_to_y(p->img_conv, 0, LV_ANIM_OFF);
}

static void slot_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    photos_t * p = (photos_t *)lv_event_get_user_data(e);

    if(code == LV_EVENT_PRESSED) {
        lv_indev_get_point(lv_indev_get_act(), &p->press_pt);
    }
    else if(code == LV_EVENT_RELEASED) {
        lv_point_t released;
        lv_indev_get_point(lv_indev_get_act(), &released);
        /* 按下和松开在同一位置 = 一次点击 → 切换全屏（原代码同样的判断） */
        if(released.x == p->press_pt.x && released.y == p->press_pt.y) {
            p->full_screen = !p->full_screen;
            if(p->app_mgr) app_manager_set_full_screen((app_manager_t *)p->app_mgr, p->full_screen);
        }
    }
}

lv_obj_t * photos_app_create(lv_obj_t * parent, void * app_mgr)
{
    photos_t * p = (photos_t *)lv_mem_alloc(sizeof(photos_t));
    lv_memset_00(p, sizeof(photos_t));
    p->app_mgr = app_mgr;

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    p->img_conv = ui_plain_obj(page, lv_pct(100), lv_pct(100));
    lv_obj_center(p->img_conv);
    lv_obj_add_flag(p->img_conv, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(p->img_conv, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_set_scroll_dir(p->img_conv, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(p->img_conv, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(p->img_conv, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(p->img_conv, img_conv_event_cb, LV_EVENT_SCROLL_END, p);

    /* 三个整屏槽位：当前 / 上 / 下 */
    for(int i = 0; i < 3; i++) {
        p->slot[i] = ui_plain_obj(p->img_conv, lv_pct(100), lv_pct(100));
        if(i == 0)      lv_obj_center(p->slot[i]);
        else if(i == 1) lv_obj_align_to(p->slot[i], p->slot[0], LV_ALIGN_OUT_TOP_MID, 0, 0);
        else            lv_obj_align_to(p->slot[i], p->slot[0], LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

        lv_obj_add_flag(p->slot[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(p->slot[i], slot_event_cb, LV_EVENT_PRESSED, p);
        lv_obj_add_event_cb(p->slot[i], slot_event_cb, LV_EVENT_RELEASED, p);

        p->img[i] = lv_img_create(p->slot[i]);
        lv_obj_align(p->img[i], LV_ALIGN_CENTER, -1, -1);
    }

    p->index = 0;
    photos_reload(p);

    return page;
}
