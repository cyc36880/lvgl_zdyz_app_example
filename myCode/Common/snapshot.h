/**
 * @file snapshot.h
 * 把当前界面截图存成 BMP（仅用于开发自检，不影响正常使用）。
 *
 * 用法：设置环境变量 MYGUI_SNAPSHOT_DIR=某个目录，程序会打开"演示脚本"模式，
 * 自动走一遍 锁屏 → 解锁 → 打开 App → 关闭 App → 提示框，并各存一张图，然后退出。
 */
#ifndef MYCODE_SNAPSHOT_H
#define MYCODE_SNAPSHOT_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void snapshot_init(void);
bool snapshot_enabled(void);
const char * snapshot_dir(void);

/** 把 lv_scr_act() 整屏存成 <snapshot_dir>/<name>.bmp，成功返回 true。 */
bool snapshot_save(const char * name);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_SNAPSHOT_H */
