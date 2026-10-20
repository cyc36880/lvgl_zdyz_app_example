/**
 * @file tester_app.h
 * 硬件测试 App（对应原 main.py 的 AppTester）：4 个竖条拖动控制
 * LED R / LED B / 蜂鸣器音量 / 蜂鸣器频率，图标会随百分比发光。
 * PC 上没有这些外设，所以"发光"就是唯一的反馈（效果部分本身是纯 UI，和板子上一致）。
 */
#ifndef MYCODE_APP_TESTER_H
#define MYCODE_APP_TESTER_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * tester_app_create(lv_obj_t * parent, void * app_mgr);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_TESTER_H */
