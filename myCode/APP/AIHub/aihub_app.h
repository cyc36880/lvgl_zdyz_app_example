/**
 * @file aihub_app.h
 * AI 例程中心（对应原 main.py 的 AppAIHub）。
 * 原版左边是 AI Demo 列表、右边是例程截图，点进去会"重启设备并替换 main.py"
 * 来跑对应的例程 —— 这件事在 PC 上做不了，所以这里保留列表 + 预览，
 * 点 Run 弹一个说明框。
 */
#ifndef MYCODE_APP_AIHUB_H
#define MYCODE_APP_AIHUB_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * aihub_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_AIHUB_H */
