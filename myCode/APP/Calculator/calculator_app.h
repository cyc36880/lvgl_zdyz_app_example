/**
 * @file calculator_app.h
 * 计算器 App（对应原 main.py 的 AppCalculator）。
 * 4x5 的网格键盘 + 右上角对齐的大号显示区 + 上方灰色的算式历史。
 */
#ifndef MYCODE_APP_CALCULATOR_H
#define MYCODE_APP_CALCULATOR_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * calculator_app_create(lv_obj_t * parent, void * user_ctx);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_CALCULATOR_H */
