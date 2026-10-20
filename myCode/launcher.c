/**
 * @file launcher.c
 * 把各个部件组装起来，并驱动时间刷新 / 键盘快捷方式 / 自检脚本。
 *
 * 这一层相当于原 main.py 的 main() + GUI.__init__()：只负责"搭骨架"，不画具体控件。
 *
 * 层次结构（和原 main.py 一致）：
 *
 *   lv_scr_act()                      整个窗口
 *    └── screen                       满屏容器，圆角 + 裁剪，模拟设备屏幕
 *         ├── home_screen.root         上白下黑渐变背景
 *         │    ├── app_conv            图标区（两页横向滑动）
 *         │    └── dock                底部 Dock
 *         ├── status_bar               顶部状态栏
 *         ├── app_manager.masker       App 的缩放容器（在 App 打开时 move_foreground）
 *         └── lock_screen.conv         锁屏（默认在最上层，解锁时向上滑出）
 */
#include "myCode/launcher.h"

#include "myCode/Common/theme.h"
#include "myCode/Common/assets.h"
#include "myCode/Common/clock_model.h"
#include "myCode/Common/snapshot.h"
#include "myCode/GUI/widgets.h"
#include "myCode/GUI/home_screen.h"
#include "myCode/GUI/lock_screen.h"
#include "myCode/GUI/status_bar.h"
#include "myCode/GUI/app_manager.h"
#include "myCode/GUI/msg_box.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* LVGL 的扩展功能需要显式初始化 */
#include "lvgl/src/extra/libs/png/lv_png.h"
#include "lvgl/src/extra/others/snapshot/lv_snapshot.h"

/* 键盘快捷方式直接读 SDL 的按键状态（PC 模拟器专有的便利功能） */
#include <SDL2/SDL.h>

static lv_obj_t *      s_screen;
static home_screen_t * s_home;
static lock_screen_t * s_lock;

/* ===================== 时间 ===================== */

static void update_time(void)
{
    /* 时间统一由 Common/clock_model 提供（真实时间 + 用户偏移），
     * Settings App 里改的就是它，锁屏 / 状态栏 / Clock App 都从这里读。 */
    struct tm tmv;
    clock_model_get(&tmv);
    lock_screen_set_time(s_lock, tmv.tm_hour, tmv.tm_min);
    home_screen_set_time(s_home, tmv.tm_hour, tmv.tm_min);
}

static void time_timer_cb(lv_timer_t * t)
{
    (void)t;
    update_time();
}

/* ===================== 锁屏 ===================== */

static void do_lock(void)
{
    if(lock_screen_is_visible(s_lock)) return;
    lock_screen_show(s_lock);
}

/* ===================== 键盘快捷方式 ===================== */

static void keys_timer_cb(lv_timer_t * t)
{
    (void)t;
    const Uint8 * ks = SDL_GetKeyboardState(NULL);
    if(ks == NULL) return;

    static bool prev_l, prev_esc;

    bool l = ks[SDL_SCANCODE_L] != 0;
    if(l && !prev_l) do_lock();
    prev_l = l;

    bool esc = ks[SDL_SCANCODE_ESCAPE] != 0;
    if(esc && !prev_esc) {
        app_manager_t * mgr = home_screen_app_manager(s_home);
        if(app_manager_is_open(mgr)) app_manager_close(mgr);
        else msg_box_close();
    }
    prev_esc = esc;
}

/* ===================== 自检脚本（截图） ===================== */

typedef struct {
    uint32_t    at_ms;
    int         action;    /* 见下面的 ACT_xxx */
    const char * arg;      /* 截图名字 / App 名字 / 页码 */
} demo_step_t;

enum { ACT_SHOT = 0, ACT_UNLOCK, ACT_OPEN, ACT_CLOSE, ACT_CLOSE_ANIM, ACT_PAGE, ACT_QUIT };

static const demo_step_t s_demo_steps[] = {
    /* 锁屏 → 解锁 */
    {  700, ACT_SHOT,       "01_lock_screen"       },
    { 1600, ACT_SHOT,       "01b_lock_screen_anim" },
    { 1700, ACT_UNLOCK,     NULL                   },
    { 1790, ACT_SHOT,       "01c_unlocking"        },
    { 2100, ACT_SHOT,       "02a_home_page0"       },

    /* App 打开 / 关闭动画 */
    { 2200, ACT_OPEN,       "Clock"                },
    { 2290, ACT_SHOT,       "03a_app_opening"      },
    { 2600, ACT_SHOT,       "03b_app_clock"        },
    { 2800, ACT_CLOSE_ANIM, NULL                   },
    { 2890, ACT_SHOT,       "04a_app_closing"      },
    { 3100, ACT_SHOT,       "04b_home_restored"    },

    /* 主屏翻页 */
    { 3200, ACT_PAGE,       "1"                    },
    { 3600, ACT_SHOT,       "02b_home_page1"       },
    { 3700, ACT_PAGE,       "0"                    },

    /* 逐个把 App 打开截一张 */
    { 3900, ACT_OPEN,       "Calculator"           },
    { 4400, ACT_SHOT,       "10_calculator"        },
    { 4500, ACT_CLOSE,      NULL                   },
    { 4600, ACT_OPEN,       "Photos"               },
    { 5100, ACT_SHOT,       "11_photos"            },
    { 5200, ACT_CLOSE,      NULL                   },
    { 5300, ACT_OPEN,       "Settings"             },
    { 5800, ACT_SHOT,       "12_settings"          },
    { 5900, ACT_CLOSE,      NULL                   },
    { 6000, ACT_OPEN,       "Freeform"             },
    { 6500, ACT_SHOT,       "13_freeform"          },
    { 6600, ACT_CLOSE,      NULL                   },
    { 6700, ACT_OPEN,       "Tester"               },
    { 7200, ACT_SHOT,       "14_tester"            },
    { 7300, ACT_CLOSE,      NULL                   },
    { 7400, ACT_OPEN,       "AI Hub"               },
    { 7900, ACT_SHOT,       "15_ai_hub"            },
    { 8000, ACT_CLOSE,      NULL                   },
    { 8100, ACT_OPEN,       "Template"             },
    { 8600, ACT_SHOT,       "16_template"          },

    /* 原版只有图标、没有实现的 8 个（这里补成了示例页） */
    { 8700, ACT_CLOSE,      NULL                   },
    { 8800, ACT_OPEN,       "Books"                },
    { 9200, ACT_SHOT,       "17_books"             },
    { 9300, ACT_CLOSE,      NULL                   },
    { 9400, ACT_OPEN,       "Files"                },
    { 9800, ACT_SHOT,       "18_files"             },
    { 9900, ACT_CLOSE,      NULL                   },
    {10000, ACT_OPEN,       "Calendar"             },
    {10400, ACT_SHOT,       "19_calendar"          },
    {10500, ACT_CLOSE,      NULL                   },
    {10600, ACT_OPEN,       "Health"               },
    {11000, ACT_SHOT,       "20_health"            },
    {11100, ACT_CLOSE,      NULL                   },
    {11200, ACT_OPEN,       "Home"                 },
    {11600, ACT_SHOT,       "21_home"              },
    {11700, ACT_CLOSE,      NULL                   },
    {11800, ACT_OPEN,       "Music"                },
    {12200, ACT_SHOT,       "22_music"             },
    {12300, ACT_CLOSE,      NULL                   },
    {12400, ACT_OPEN,       "Notes"                },
    {12800, ACT_SHOT,       "23_notes"             },
    {12900, ACT_CLOSE,      NULL                   },
    {13000, ACT_OPEN,       "Weather"              },
    {13400, ACT_SHOT,       "24_weather"           },
    {13500, ACT_QUIT,       NULL                   },
};

#define DEMO_STEP_NUM ((int)(sizeof(s_demo_steps) / sizeof(s_demo_steps[0])))

static uint32_t s_demo_t0;
static int      s_demo_idx;

static void demo_timer_cb(lv_timer_t * t)
{
    uint32_t elapsed = lv_tick_elaps(s_demo_t0);

    while(s_demo_idx < DEMO_STEP_NUM && elapsed >= s_demo_steps[s_demo_idx].at_ms) {
        const demo_step_t * step = &s_demo_steps[s_demo_idx];
        switch(step->action) {
            case ACT_SHOT:
                snapshot_save(step->arg);
                break;
            case ACT_UNLOCK:
                lock_screen_force_unlock(s_lock);
                break;
            case ACT_OPEN:
                app_manager_open_by_name(home_screen_app_manager(s_home), step->arg);
                break;
            case ACT_CLOSE:
                app_manager_close(home_screen_app_manager(s_home));
                break;
            case ACT_CLOSE_ANIM:
                app_manager_close_animated(home_screen_app_manager(s_home));
                break;
            case ACT_PAGE:
                home_screen_scroll_page(s_home, step->arg ? atoi(step->arg) : 0);
                break;
            case ACT_QUIT:
                lv_timer_del(t);
                printf("[mygui] demo finished, snapshots in %s\n", snapshot_dir());
                fflush(stdout);
                exit(0);
            default:
                break;
        }
        s_demo_idx++;
    }
}

/* ===================== 入口 ===================== */

void my_launcher_init(void)
{
    assets_init();
    clock_model_init();
    snapshot_init();
    lv_png_init();          /* 注册 PNG 解码器，否则 RAW_ALPHA 的 PNG 数据没法用 */

    /* 最外层窗口是黑的，模拟设备边框 */
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* device screen：圆角 + 裁剪，所有界面都放在里面 */
    s_screen = ui_plain_obj(scr, lv_pct(100), lv_pct(100));
    lv_obj_align(s_screen, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_radius(s_screen, UI_SCREEN_RADIUS, 0);
    lv_obj_set_style_clip_corner(s_screen, true, 0);
    lv_obj_set_style_bg_color(s_screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_screen, LV_OPA_COVER, 0);

    /* 先建主屏，再建锁屏，锁屏自然在最上层 */
    s_home = home_screen_create(s_screen, NULL);
    s_lock = lock_screen_create(s_screen, NULL, NULL);

    update_time();

    lv_timer_create(time_timer_cb, 1000, NULL);
    lv_timer_create(keys_timer_cb, 50, NULL);

    /* 自检脚本模式 */
    if(snapshot_enabled()) {
        s_demo_t0  = lv_tick_get();
        s_demo_idx = 0;
        lv_timer_create(demo_timer_cb, 30, NULL);
    }

    printf("[mygui] asset root: %s\n", assets_root());
    fflush(stdout);
}
