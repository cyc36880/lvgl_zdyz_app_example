/**
 * @file extras_app.h
 * 原版 main.py 里"只有图标、没有实现"的 8 个入口（app_dict 里查不到它们，
 * 点开会弹 "The app has not been implemented yet"）。
 *
 * 为了让主屏每个图标都能点进去，这里给它们各做了一个简短但真的能动的示例页。
 * 8 个都很小，所以放在同一个文件里；想单独扩展的话，照着 APP/Clock/ 那样拆出去即可。
 */
#ifndef MYCODE_APP_EXTRAS_H
#define MYCODE_APP_EXTRAS_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * books_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * files_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * calendar_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * health_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * home_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * music_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * notes_app_create(lv_obj_t * parent, void * app_mgr);
lv_obj_t * weather_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_EXTRAS_H */
