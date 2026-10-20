/**
 * @file app_manager.h
 * 应用图标 + App 打开/关闭动画（原 main.py 的 AppManager / AppLoader / AppBase）。
 *
 * 核心思路（原代码有，这里原样复刻）：
 *   - 每个图标按下时放大 256→270，松开回 256（单纯的手感反馈）。
 *   - 点开 App 时，一个叫 masker 的容器从"图标所在的矩形"（宽高 1）放大到整个屏幕，
 *     App 内容放在 masker 里，所以看上去就是"从图标里长出来"。
 *   - 在 App 里从底部往上拖，masker 会边拖边缩小；松手时如果拖过屏幕 3/4 就缩回图标，
 *     否则弹回全屏。缩到图标大小后销毁 App，露出主屏。
 */
#ifndef MYCODE_GUI_APP_MANAGER_H
#define MYCODE_GUI_APP_MANAGER_H

#include "lvgl/lvgl.h"
#include "myCode/GUI/status_bar.h"

#ifdef __cplusplus
extern "C" {
#endif

/** App 内容构造函数：往 parent 里创建界面即可，返回值可以为 NULL。
 *  第二个参数就是 app_manager_t*，App 想全屏 / 关闭自己时用它。 */
typedef lv_obj_t * (*app_create_cb_t)(lv_obj_t * parent, void * app_mgr);

/** 一个应用条目的描述（名字、图标、内容构造函数）。 */
typedef struct {
    const char *    name;         /* 显示在图标下方的文字，同时是查找用的 key */
    const char *    icon;         /* 资源相对路径，例如 "APP/icons/xxx.png" */
    app_create_cb_t create;       /* NULL 表示"还没实现"，点开会弹提示框 */
    bool            full_screen;  /* true: 打开时隐藏状态栏并下移 home bar */
    bool            dark_bg;      /* true: App 背景是深色，状态栏文字改白色 */
} app_entry_t;

typedef struct app_manager app_manager_t;

/**
 * @param screen     承载 masker 的容器（一般就是满屏的那个 device screen）
 * @param status_bar 状态栏，App 全屏时要用
 * @param user_ctx   透传给 app_create_cb_t 的上下文
 */
app_manager_t * app_manager_create(lv_obj_t * screen, status_bar_t * status_bar, void * user_ctx);

/** 在 slot 里创建一个图标 + 标签，并登记到图标索引表。
 *  label_color 由调用者给：主屏上排用黑色、下排/Dock 用白色（配合上白下黑的渐变背景）。 */
void app_manager_make_icon(app_manager_t * mgr, lv_obj_t * slot, const app_entry_t * entry,
                           lv_color_t label_color);

/** 打开指定 App（icon_area 是图标矩形，动画起点）。 */
void app_manager_open(app_manager_t * mgr, const app_entry_t * entry, const lv_area_t * icon_area);

/** 按名字打开（给自检脚本/键盘快捷方式用）。返回 false 表示没找到图标。 */
bool app_manager_open_by_name(app_manager_t * mgr, const char * name);

/** 关掉当前 App（按键 ESC 时用到），立即销毁，无动画。 */
void app_manager_close(app_manager_t * mgr);

/** 带"缩回图标"动画地关闭（拖拽关闭走的就是这条路）。 */
void app_manager_close_animated(app_manager_t * mgr);

/** App 内部切换全屏（对应原代码的 status_bar.enter_full_screen() / exit_full_screen()）：
 *  全屏时状态栏上移隐藏、App 的 home bar 下移 22px。 */
void app_manager_set_full_screen(app_manager_t * mgr, bool full_screen);

bool app_manager_is_open(app_manager_t * mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_GUI_APP_MANAGER_H */
