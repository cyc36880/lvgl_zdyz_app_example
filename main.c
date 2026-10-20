/**
 * @file main.c
 * @brief PC 模拟器入口：初始化 LVGL + SDL 后端，然后交给 myCode 的启动器。
 *
 * 分工：
 *   main.c               LVGL 内核、SDL 显示/输入设备、主循环（"板级"代码）
 *   myCode/launcher.c    my_launcher_init() 搭建整个桌面界面
 */
#include <stdio.h>
#include <stdbool.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/sdl/sdl.h"
#include "myCode/launcher.h"

/**********************
 *  静态函数声明
 **********************/
static void hal_init(void);
static void lvgl_loop(void);

/**********************
 *  HAL 初始化
 *
 *  配置 SDL2 驱动的 LVGL 显示和输入设备：
 *  - 显示：640x480 窗口（尺寸见 include/lv_drv_conf.h 的 SDL_HOR_RES/VER_RES）
 *  - 输入：鼠标指针、键盘按键、鼠标滚轮编码器
 *  - 导航：默认 group 支持键盘 Tab/方向键切换焦点
 *  - 光标：自定义图片替代系统光标
 **********************/
static void hal_init(void)
{
    /* 初始化 SDL（创建窗口、渲染器、纹理在 sdl_init 内部完成） */
    sdl_init();

    /* 显示缓冲：整屏一块 buffer，简单可靠 */
    static lv_disp_draw_buf_t disp_buf1;
    static lv_color_t buf1_1[SDL_HOR_RES * SDL_VER_RES];
    lv_disp_draw_buf_init(&disp_buf1, buf1_1, NULL,
                          sizeof(buf1_1) / sizeof(lv_color_t));

    /* 注册显示驱动 */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf1;
    disp_drv.flush_cb = sdl_display_flush;    /* SDL 纹理刷新回调 */
    disp_drv.hor_res  = SDL_HOR_RES;
    disp_drv.ver_res  = SDL_VER_RES;

    lv_disp_t * disp = lv_disp_drv_register(&disp_drv);

    /* 暗色主题：蓝色主色调、红色强调色 */
    lv_theme_t * th = lv_theme_default_init(
        disp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true,
        LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    /* 默认 group：键盘导航（Tab/方向键切换焦点）的必要前提 */
    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);

    /* 注册鼠标输入 */
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1);
    indev_drv_1.type    = LV_INDEV_TYPE_POINTER;
    indev_drv_1.read_cb = sdl_mouse_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv_1);

    /* 注册键盘输入 */
    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2);
    indev_drv_2.type    = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = sdl_keyboard_read;
    lv_indev_t * kb_indev = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);

    /* 注册滚轮输入（作为编码器） */
    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3);
    indev_drv_3.type    = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = sdl_mousewheel_read;
    lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
    lv_indev_set_group(enc_indev, g);

    /* 替换系统光标为自定义图片 */
    LV_IMG_DECLARE(mouse_cursor_icon);
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);
    lv_indev_set_cursor(mouse_indev, cursor_obj);

    printf("[HAL] init done\n");
    fflush(stdout);
}

/**********************
 *  主循环
 *
 *  每一帧 lv_timer_handler() 驱动 LVGL 的定时器、动画、输入和重绘，
 *  然后小睡 1ms 让出 CPU（同时保持对 SDL 事件的响应）。
 *
 *  sdl_quit_qry 由 lv_drivers 的事件处理在收到 SDL_QUIT 时置位。
 **********************/
static void lvgl_loop(void)
{
    printf("[App] enter loop...\n");
    fflush(stdout);

    extern volatile bool sdl_quit_qry;

    while(!sdl_quit_qry) {
        lv_timer_handler();
        SDL_Delay(1);
    }

    printf("[App] loop exit\n");
    fflush(stdout);
}

/**********************
 *  主函数
 **********************/
int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    /* 初始化 LVGL 内核（内存、日志、定时器等） */
    lv_init();

    /* 初始化 HAL 层（SDL2 显示、鼠标、键盘、滚轮） */
    hal_init();

    /* 搭建桌面界面（锁屏 → 主屏 → App） */
    my_launcher_init();

    /* 进入主循环，直至窗口关闭 */
    lvgl_loop();

    printf("[App] cleanup done\n");
    return 0;
}
