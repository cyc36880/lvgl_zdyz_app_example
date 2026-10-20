/**
 * @file photos_app.h
 * 相册 App（对应原 main.py 的 AppPhotos）。
 * 上下滑动翻照片，点击画面切换全屏。
 */
#ifndef MYCODE_APP_PHOTOS_H
#define MYCODE_APP_PHOTOS_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * photos_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_PHOTOS_H */
