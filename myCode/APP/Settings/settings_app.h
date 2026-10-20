/**
 * @file settings_app.h
 * 设置 App（对应原 main.py 的 AppSettings）：左边是栏（头像卡 + 系统项），
 * 右边是具体设置页；点 "Date & Time" 在右边生成年月日 / 时分秒的滚轮。
 */
#ifndef MYCODE_APP_SETTINGS_H
#define MYCODE_APP_SETTINGS_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * settings_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_SETTINGS_H */
