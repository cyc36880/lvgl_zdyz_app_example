/**
 * @file lv_drv_conf.h
 * @brief PC 模拟器专用的 lv_drivers 配置（lv_drivers 的 SDL 后端）。
 *
 * 只启用 SDL 驱动（显示 + 鼠标 + 滚轮 + 键盘），屏幕尺寸按 myCode 的界面设计
 * 设为 640x480（与 CanMV K230 的屏一致）。
 *
 * 注意：本文件通过 -DLV_CONF_INCLUDE_SIMPLE 被 lv_drivers/sdl/sdl.h 以
 * #include "lv_drv_conf.h" 的方式找到，所以必须放在 include/ 目录下。
 */
#ifndef LV_DRV_CONF_H
#define LV_DRV_CONF_H

/*-------------------
 *  SDL
 *-------------------*/

/* SDL based drivers for display, mouse, mousewheel and keyboard */
#ifndef USE_SDL
#  define USE_SDL 1
#endif

/* Hardware accelerated SDL driver */
#ifndef USE_SDL_GPU
#  define USE_SDL_GPU 0
#endif

#if USE_SDL || USE_SDL_GPU
#  define SDL_HOR_RES     640
#  define SDL_VER_RES     480

/* Scale window by this factor (useful when simulating small screens) */
#  define SDL_ZOOM        1

/* Used to test true double buffering with only address changing.
 * Use 2 draw buffers, bith with SDL_HOR_RES x SDL_VER_RES size*/
#  define SDL_DOUBLE_BUFFERED 0

/*Eclipse: <SDL2/SDL.h>    Visual Studio: <SDL.h>*/
#  define SDL_INCLUDE_PATH    <SDL2/SDL.h>

/*Open two windows to test multi display support*/
#  define SDL_DUAL_DISPLAY            0
#endif

/*-------------------
 *  Monitor of PC (deprecated)
 *-------------------*/
#ifndef USE_MONITOR
#  define USE_MONITOR 0
#endif

/*---------------------------------------
 * Mouse / mousewheel / keyboard (deprecated)
 *
 * 这三项只是旧接口：USE_SDL 打开时 SDL 驱动已经同时提供
 * 显示、鼠标、滚轮和键盘，所以这里保持关闭。
 *-------------------------------------*/
#ifndef USE_MOUSE
#  define USE_MOUSE           0
#endif

#ifndef USE_MOUSEWHEEL
#  define USE_MOUSEWHEEL      0
#endif

#ifndef USE_KEYBOARD
#  define USE_KEYBOARD        0
#endif

#endif /* LV_DRV_CONF_H */
