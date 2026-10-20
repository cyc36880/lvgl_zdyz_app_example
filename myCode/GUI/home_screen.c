/**
 * @file home_screen.c
 */
#include "myCode/GUI/home_screen.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"
#include "myCode/APP/AIHub/aihub_app.h"
#include "myCode/APP/Calculator/calculator_app.h"
#include "myCode/APP/Clock/clock_app.h"
#include "myCode/APP/Extras/extras_app.h"
#include "myCode/APP/Freeform/freeform_app.h"
#include "myCode/APP/Photos/photos_app.h"
#include "myCode/APP/Settings/settings_app.h"
#include "myCode/APP/Template/template_app.h"
#include "myCode/APP/Tester/tester_app.h"

#define GRID_H      280    /* 图标区域高度 */
#define GRID_ROW_H  140    /* 一行的高度 */
#define PAGE_NUM    2

struct home_screen {
    lv_obj_t *      root;       /* 渐变背景容器 */
    status_bar_t *  status_bar;
    app_manager_t * app_mgr;
    lv_obj_t *      app_conv;   /* 可横向滑动的图标容器 */
    lv_obj_t *      dock;
    lv_obj_t *      pages[PAGE_NUM];
};

/* ---------- 应用表：对应 main.py 末尾 main() 里的 add_app 调用 ----------
 * 图标路径的写法和 CanMV 一样：RESOURCES_PATH + "APP/icons/xxx.png"，
 * 这里的资源根目录就是 myCode/，所以直接写 "APP/icons/xxx.png"。
 *
 * 最后两个 bool：full_screen（打开时隐藏状态栏）、dark_bg（状态栏改白字）。 */

static const app_entry_t s_app_settings   = { "Settings",   "APP/icons/app_icon_90x90_settings.png",
                                              settings_app_create,   false, false };
static const app_entry_t s_app_clock      = { "Clock",      "APP/icons/app_icon_90x90_clock.png",
                                              clock_app_create,      false, true  };
static const app_entry_t s_app_calculator = { "Calculator", "APP/icons/app_icon_90x90_calculator.png",
                                              calculator_app_create, false, true  };
static const app_entry_t s_app_ai_hub     = { "AI Hub",     "APP/icons/app_icon_90x90_intelligence.png",
                                              aihub_app_create,      false, false };

static const app_entry_t s_app_photos     = { "Photos",     "APP/icons/app_icon_90x90_photos.png",
                                              photos_app_create,     false, true  };
static const app_entry_t s_app_freeform   = { "Freeform",   "APP/icons/app_icon_90x90_freeform.png",
                                              freeform_app_create,   false, false };
static const app_entry_t s_app_tester     = { "Tester",     "APP/icons/app_icon_90x90_test_flight.png",
                                              tester_app_create,     false, false };
static const app_entry_t s_app_template   = { "Template",   "APP/icons/app_icon_90x90_template.png",
                                              template_app_create,   false, false };

/* 这 8 个在 CanMV 的 app_dict 里没有实现（原版点开只弹提示），
 * 这里各补了一个简短但可用的示例页，见 APP/Extras/extras_app.c */
static const app_entry_t s_app_books      = { "Books",      "APP/icons/app_icon_90x90_books.png",
                                              books_app_create,      false, false };
static const app_entry_t s_app_files      = { "Files",      "APP/icons/app_icon_90x90_files.png",
                                              files_app_create,      false, false };
static const app_entry_t s_app_calendar   = { "Calendar",   "APP/icons/app_icon_90x90_calendar.png",
                                              calendar_app_create,   false, false };
static const app_entry_t s_app_health     = { "Health",     "APP/icons/app_icon_90x90_health.png",
                                              health_app_create,     false, false };

static const app_entry_t s_app_home       = { "Home",       "APP/icons/app_icon_90x90_home.png",
                                              home_app_create,       false, false };
static const app_entry_t s_app_music      = { "Music",      "APP/icons/app_icon_90x90_music.png",
                                              music_app_create,      false, false };
static const app_entry_t s_app_notes      = { "Notes",      "APP/icons/app_icon_90x90_notes.png",
                                              notes_app_create,      false, false };
static const app_entry_t s_app_weather    = { "Weather",    "APP/icons/app_icon_90x90_weather.png",
                                              weather_app_create,    false, false };

/* ---------- 布局小工具 ---------- */

static lv_obj_t * make_row(lv_obj_t * page)
{
    lv_obj_t * row = ui_plain_obj(page, lv_pct(100), GRID_ROW_H);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return row;
}

static lv_obj_t * make_slot(lv_obj_t * row)
{
    lv_obj_t * slot = ui_plain_obj(row, UI_ICON_SLOT_W, UI_ICON_SLOT_H);
    return slot;
}

static lv_obj_t * make_page(lv_obj_t * app_conv)
{
    lv_obj_t * page = ui_plain_obj(app_conv, lv_pct(100), GRID_H);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return page;
}

home_screen_t * home_screen_create(lv_obj_t * parent, void * user_ctx)
{
    home_screen_t * hs = (home_screen_t *)lv_mem_alloc(sizeof(home_screen_t));
    lv_memset_00(hs, sizeof(home_screen_t));

    /* 背景：上白下黑的竖直渐变（原代码的 conv.set_style_bg_grad_color / GRAD_DIR.VER） */
    hs->root = ui_gradient_bg(parent, lv_pct(100), lv_pct(100),
                              UI_COLOR_BG_TOP, UI_COLOR_BG_BOTTOM, 0);
    lv_obj_align(hs->root, LV_ALIGN_TOP_LEFT, 0, 0);

    /* 状态栏挂在最外层，和 App 的 masker 是兄弟节点 */
    hs->status_bar = status_bar_create(parent);

    /* App 管理器也挂最外层：它的 masker 要能盖住图标区、并能 move_foreground 盖住状态栏 */
    hs->app_mgr = app_manager_create(parent, hs->status_bar, user_ctx);

    /* ---- 图标区：两页横向滑动 ---- */
    hs->app_conv = ui_plain_obj(hs->root, lv_pct(100), GRID_H);
    lv_obj_align(hs->app_conv, LV_ALIGN_CENTER, 0, -60);
    lv_obj_add_flag(hs->app_conv, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(hs->app_conv, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(hs->app_conv, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(hs->app_conv, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * page0 = make_page(hs->app_conv);
    lv_obj_align(page0, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * page1 = make_page(hs->app_conv);
    lv_obj_align_to(page1, page0, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);

    hs->pages[0] = page0;
    hs->pages[1] = page1;

    /* 第 0 页：上排 4 个（黑字，因为背景靠上偏白） */
    lv_obj_t * row = make_row(page0);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_photos,   UI_COLOR_TEXT_DARK);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_freeform, UI_COLOR_TEXT_DARK);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_tester,   UI_COLOR_TEXT_DARK);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_template, UI_COLOR_TEXT_DARK);

    /* 第 1 页：上排黑字、下排白字（背景越往下越黑，和原代码的颜色分配一致） */
    row = make_row(page1);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_books,    UI_COLOR_TEXT_DARK);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_files,    UI_COLOR_TEXT_DARK);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_calendar, UI_COLOR_TEXT_DARK);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_health,   UI_COLOR_TEXT_DARK);

    row = make_row(page1);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_home,     UI_COLOR_TEXT_LIGHT);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_music,    UI_COLOR_TEXT_LIGHT);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_notes,    UI_COLOR_TEXT_LIGHT);
    app_manager_make_icon(hs->app_mgr, make_slot(row), &s_app_weather,  UI_COLOR_TEXT_LIGHT);

    /* ---- 底部 Dock ---- */
    hs->dock = ui_plain_obj(hs->root, lv_pct(90), UI_DOCK_H);
    lv_obj_align(hs->dock, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_radius(hs->dock, 25, 0);
    lv_obj_set_style_clip_corner(hs->dock, true, 0);
    lv_obj_set_style_bg_color(hs->dock, UI_COLOR_BG_TOP, 0);
    lv_obj_set_style_bg_opa(hs->dock, LV_OPA_50, 0);
    lv_obj_set_flex_flow(hs->dock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hs->dock, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    app_manager_make_icon(hs->app_mgr, make_slot(hs->dock), &s_app_settings,   UI_COLOR_TEXT_LIGHT);
    app_manager_make_icon(hs->app_mgr, make_slot(hs->dock), &s_app_clock,      UI_COLOR_TEXT_LIGHT);
    app_manager_make_icon(hs->app_mgr, make_slot(hs->dock), &s_app_calculator, UI_COLOR_TEXT_LIGHT);
    app_manager_make_icon(hs->app_mgr, make_slot(hs->dock), &s_app_ai_hub,     UI_COLOR_TEXT_LIGHT);

    return hs;
}

void home_screen_set_time(home_screen_t * hs, int hour, int minute)
{
    if(hs == NULL) return;
    status_bar_set_time(hs->status_bar, hour, minute);
}

void home_screen_scroll_page(home_screen_t * hs, int page)
{
    if(hs == NULL || page < 0 || page >= PAGE_NUM) return;
    lv_obj_scroll_to_view(hs->pages[page], LV_ANIM_ON);
}

status_bar_t * home_screen_status_bar(home_screen_t * hs)
{
    return hs ? hs->status_bar : NULL;
}

app_manager_t * home_screen_app_manager(home_screen_t * hs)
{
    return hs ? hs->app_mgr : NULL;
}
