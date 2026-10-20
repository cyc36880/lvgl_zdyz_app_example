/**
 * @file template_app.h
 * 最简单的示例 App（对应原 main.py 的 AppTemplate）：白底 + 一个按钮。
 */
#ifndef MYCODE_APP_TEMPLATE_H
#define MYCODE_APP_TEMPLATE_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** App 构造函数：往 parent 里画界面即可。 */
lv_obj_t * template_app_create(lv_obj_t * parent, void * user_ctx);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_APP_TEMPLATE_H */
