/**
 * @file aihub_app.c
 *
 * 原 main.py 的 AppAIHub 结构：
 *   DemoManager 在左侧列表里 add(icon, 名字, 中文说明, 截图, 脚本路径)
 *   右侧 demo_conv 显示选中例程的截图
 *   点进去弹 RebootConfirmer（"确认重启设备运行 Demo?" Cancel / Confirm），
 *   Confirm 会 machine.reset() 并把 main.py 换成该例程的脚本。
 *
 * PC 上既没有 K230 也没有摄像头，所以：列表 + 截图预览保留，Run 改成弹说明框。
 */
#include "myCode/APP/AIHub/aihub_app.h"
#include "myCode/Common/assets.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"
#include "myCode/GUI/msg_box.h"

#include <stdio.h>

#define ICON_SIZE 56

typedef struct {
    const char * name;     /* 英文名（列表标题） */
    const char * desc;     /* 一行说明 */
    const char * image;    /* 截图资源路径 */
} demo_entry_t;

/* 顺序和原 main.py 里一连串 demo_manager.add(...) 对应。
 * 说明文字用英文：内置的 simsun CJK 字体只收录了常用字，中文很容易缺字。 */
static const demo_entry_t s_demos[] = {
    { "Face Detection",          "detect faces",        "APP/AIHub/face_detection_show.png" },
    { "Face Landmark",           "face key points",     "APP/AIHub/face_landmark_show.png" },
    { "Face Pose",               "head pose",           "APP/AIHub/face_pose_show.png" },
    { "Person Detection",        "detect people",       "APP/AIHub/person_detection_show.png" },
    { "Person Keypoint",         "body key points",     "APP/AIHub/person_keypoint_detect_show.png" },
    { "Hand Detection",          "detect hands",        "APP/AIHub/hand_detection_show.png" },
    { "Hand Keypoint Detection", "hand key points",     "APP/AIHub/hand_keypoint_detection_show.png" },
    { "Hand Keypoint Class",     "gesture classify",    "APP/AIHub/hand_keypoint_class_show.png" },
    { "Dynamic Gesture",         "moving gesture",      "APP/AIHub/dynamic_gesture_show.png" },
    { "Finger Guessing",         "rock paper scissors", "APP/AIHub/finger_guessing_show.png" },
    { "Falldown Detect",         "fall detection",      "APP/AIHub/falldown_detect_show.png" },
    { "Self Learning",           "learn by itself",     "APP/AIHub/self_learning_show.png" },
    { "Licence Det",             "plate detection",     "APP/AIHub/licence_det_show.png" },
    { "Licence Det Rec",         "plate recognition",   "APP/AIHub/licence_det_rec_show.png" },
    { "Object Detect YOLOv8n",   "yolov8n objects",     "APP/AIHub/object_detect_yolov8n_show.png" },
    { "Segmentation",            "semantic seg",        "APP/AIHub/segmentation_show.png" },
    { "Space Resize",            "resize space",        "APP/AIHub/space_resize_show.png" },
    { "Keyword Spotting",        "wake word",           "APP/AIHub/keyword_spotting_show.png" },
    { "Hand Recognition",        "gesture recognize",   "APP/AIHub/hand_recognition_show.png" },
};

#define DEMO_NUM ((int)(sizeof(s_demos) / sizeof(s_demos[0])))

typedef struct {
    lv_obj_t *    preview;        /* 右侧截图 */
    lv_obj_t *    preview_title;
    const demo_entry_t * current;
} aihub_t;

static void select_demo(aihub_t * a, const demo_entry_t * d)
{
    a->current = d;

    const lv_img_dsc_t * dsc = assets_png(d->image);
    if(dsc) lv_img_set_src(a->preview, dsc);
    lv_obj_center(a->preview);

    lv_label_set_text(a->preview_title, d->name);
}

static void item_event_cb(lv_event_t * e)
{
    aihub_t * a = (aihub_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    const demo_entry_t * d = (const demo_entry_t *)lv_obj_get_user_data(lv_event_get_target(e));
    if(d) select_demo(a, d);
}

static void run_event_cb(lv_event_t * e)
{
    aihub_t * a = (aihub_t *)lv_event_get_user_data(e);
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    char msg[160];
    snprintf(msg, sizeof(msg),
             "%s\n\nOn K230 this reboots and runs the demo script.\n"
             "PC simulator has no camera / NPU, so only the preview is available.",
             a->current ? a->current->name : "");
    msg_box_show(lv_scr_act(), "AI Demo", msg, "OK");
}

lv_obj_t * aihub_app_create(lv_obj_t * parent, void * app_mgr)
{
    (void)app_mgr;

    aihub_t * a = (aihub_t *)lv_mem_alloc(sizeof(aihub_t));
    lv_memset_00(a, sizeof(aihub_t));

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0xF3F2F7), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    /* ---- 左栏：列表 ---- */
    lv_obj_t * side_bar = ui_plain_obj(page, lv_pct(46), lv_pct(100));
    lv_obj_align(side_bar, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * title = ui_label(side_bar, UI_FONT_TITLE, lv_color_hex(0x000000), "AI Demo");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 50);

    lv_obj_t * list = ui_plain_obj(side_bar, lv_pct(90), 330);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 95);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * prev = NULL;
    for(int i = 0; i < DEMO_NUM; i++) {
        lv_obj_t * slot = ui_plain_obj(list, lv_pct(100), 76);
        if(prev == NULL) lv_obj_align(slot, LV_ALIGN_TOP_MID, 0, 0);
        else             lv_obj_align_to(slot, prev, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
        prev = slot;

        lv_obj_t * card = lv_obj_create(slot);
        lv_obj_set_size(card, lv_pct(95), 72);
        lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_pad_all(card, 0, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_style_radius(card, 16, 0);
        lv_obj_set_style_clip_corner(card, true, 0);
        lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(card, (void *)&s_demos[i]);
        lv_obj_add_event_cb(card, item_event_cb, LV_EVENT_CLICKED, a);

        lv_obj_t * icon = ui_img(card, "APP/AIHub/app_icon_72x72_intelligence.png");
        lv_img_set_zoom(icon, 160);          /* 72x72 缩到大约 45x45（256 = 100%） */
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t * name = ui_label(card, UI_FONT_SMALL, lv_color_hex(0x000000), s_demos[i].name);
        lv_obj_align_to(name, icon, LV_ALIGN_OUT_RIGHT_MID, 10, -12);

        lv_obj_t * desc = ui_label(card, UI_FONT_SMALL, lv_color_hex(0x8A8A8A), s_demos[i].desc);
        lv_obj_align_to(desc, name, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }

    /* ---- 右栏：预览 ---- */
    lv_obj_t * split = ui_plain_obj(page, 2, lv_pct(100));
    lv_obj_align_to(split, side_bar, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(split, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(split, 40, 0);

    lv_obj_t * demo_area = ui_plain_obj(page, lv_pct(54), lv_pct(100));
    lv_obj_align(demo_area, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(demo_area, lv_color_hex(0x14141A), 0);
    lv_obj_set_style_bg_opa(demo_area, LV_OPA_COVER, 0);

    a->preview_title = ui_label(demo_area, UI_FONT_NORMAL, lv_color_hex(0xFFFFFF), "");
    lv_obj_align(a->preview_title, LV_ALIGN_TOP_MID, 0, 30);

    a->preview = lv_img_create(demo_area);
    lv_obj_center(a->preview);

    lv_obj_t * run_btn = lv_btn_create(demo_area);
    lv_obj_set_size(run_btn, 160, 48);
    lv_obj_align(run_btn, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_radius(run_btn, 16, 0);
    lv_obj_set_style_bg_color(run_btn, lv_color_hex(0x007CFB), 0);
    lv_obj_set_style_shadow_width(run_btn, 0, 0);
    lv_obj_set_style_border_width(run_btn, 0, 0);
    lv_obj_add_event_cb(run_btn, run_event_cb, LV_EVENT_CLICKED, a);

    lv_obj_t * run_label = ui_label(run_btn, UI_FONT_NORMAL, lv_color_hex(0xFFFFFF), "Run");
    lv_obj_center(run_label);

    select_demo(a, &s_demos[0]);

    return page;
}
