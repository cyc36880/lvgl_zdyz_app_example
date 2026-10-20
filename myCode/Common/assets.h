/**
 * @file assets.h
 * 图片资源加载（对应 CanMV 里的 RESOURCES_PATH + open(path,'rb').read()）。
 *
 * 原 MicroPython 代码是这么干的（main.py 第 320~323 行）：
 *     with open(path, 'rb') as f: data = f.read()
 *     dsc = lv.img_dsc_t({'data_size': len(data), 'data': data})
 *     img.set_src(dsc)
 * 也就是"把 PNG 文件的原始字节交给 LVGL，让它自己解码"。
 * C 版本完全一样，只是多了一步自己读文件、并且要把内存留住（不能是栈变量）。
 */
#ifndef MYCODE_COMMON_ASSETS_H
#define MYCODE_COMMON_ASSETS_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化资源根目录。
 *
 * 资源根目录 = myCode/ 本身，所以相对路径的写法和 CanMV 完全一致：
 *     CanMV:  RESOURCES_PATH + "APP/icons/app_icon_90x90_clock.png"
 *     这里:   assets_png("APP/icons/app_icon_90x90_clock.png")
 *
 * 默认值由 CMakeLists.txt 通过 MYGUI_ASSET_DIR 宏传进来（绝对路径）；
 * 也可以用环境变量 MYGUI_ASSETS 临时覆盖。
 */
void assets_init(void);

/** 返回资源根目录（调试用）。 */
const char * assets_root(void);

/**
 * 按相对路径加载一张 PNG，返回可以被 lv_img_set_src() 使用的图像描述符。
 * 结果会被缓存，重复调用不会重复读盘；失败返回 NULL。
 * @param rel 例如 "APP/icons/app_icon_90x90_clock.png"
 */
const lv_img_dsc_t * assets_png(const char * rel);

#ifdef __cplusplus
}
#endif

#endif /* MYCODE_COMMON_ASSETS_H */
