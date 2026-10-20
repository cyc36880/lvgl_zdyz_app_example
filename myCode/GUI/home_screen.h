/**
 * @file home_screen.h
 * 主屏：渐变背景 + 顶部状态栏 + 应用图标网格（两页横向滑动）+ 底部 Dock。
 * 对应原 main.py 的 GUI.HomeScreen。
 */
#ifndef MYCODE_GUI_HOME_SCREEN_H
#define MYCODE_GUI_HOME_SCREEN_H

#include "lvgl/lvgl.h"
#include "myCode/GUI/app_manager.h"
#include "myCode/GUI/status_bar.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct home_screen home_screen_t;

/**
 * @param parent    外层满屏容器（带圆角/裁剪）
 * @param user_ctx  透传给 App 创建函数的上下文
 */
home_screen_t * home_screen_create(lv_obj_t * parent, void * user_ctx);

void home_screen_set_time(home_screen_t * hs, int hour, int minute);

/** 滚动到第 page 页（0 或 1），带动画。 */
void home_screen_scroll_page(home_screen_t * hs, int page);

status_bar_t *   home_screen_status_bar(home_screen_t * hs);
app_manager_t *  home_screen_app_manager(home_screen_t * hs);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_GUI_HOME_SCREEN_H */
