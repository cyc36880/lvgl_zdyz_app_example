/**
 * @file lock_screen.h
 * 锁屏：渐变背景 + 大号时钟 + "Swipe up to open" 呼吸动画 + 上滑解锁。
 * 对应原 main.py 的 GUI.LockScreen。
 */
#ifndef MYCODE_LOCK_SCREEN_H
#define MYCODE_LOCK_SCREEN_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lock_screen lock_screen_t;

/**
 * @param parent   外层满屏容器
 * @param on_unlock 解锁动画结束后回调（用来真正隐藏锁屏 / 通知外部）
 * @param ctx      回调上下文
 */
lock_screen_t * lock_screen_create(lv_obj_t * parent,
                                   void (*on_unlock)(void * ctx), void * ctx);

/** 重新锁屏：从上方滑下来（复刻原代码 show() 的 -480 → 0 动画）。 */
void lock_screen_show(lock_screen_t * ls);

/** 立刻隐藏（不带动画，用于初始状态）。 */
void lock_screen_hide_now(lock_screen_t * ls);

void lock_screen_set_time(lock_screen_t * ls, int hour, int minute);

bool lock_screen_is_visible(lock_screen_t * ls);

/** 供自检脚本使用：直接触发"上滑解锁"。 */
void lock_screen_force_unlock(lock_screen_t * ls);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_LOCK_SCREEN_H */
