/**
 * @file msg_box.h
 * 全屏提示框（原 main.py 的 AppManager.FullScreenMSG）。
 * 点用未实现的 App 时用来提示"还没做"。
 */
#ifndef MYCODE_MSG_BOX_H
#define MYCODE_MSG_BOX_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void msg_box_show(lv_obj_t * parent, const char * title, const char * msg, const char * ok_text);
void msg_box_close(void);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_MSG_BOX_H */
