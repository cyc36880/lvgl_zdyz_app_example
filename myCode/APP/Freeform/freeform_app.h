/**
 * @file freeform_app.h
 * 画板 App（对应原 main.py 的 AppFreeform）：整屏 lv_canvas 手写，左侧滑出画笔设置。
 */
#ifndef MYCODE_APP_FREEFORM_H
#define MYCODE_APP_FREEFORM_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * freeform_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_FREEFORM_H */
