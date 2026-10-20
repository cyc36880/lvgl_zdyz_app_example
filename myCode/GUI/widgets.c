/**
 * @file widgets.c
 */
#include "myCode/GUI/widgets.h"
#include "myCode/Common/assets.h"
#include "myCode/Common/theme.h"

/* ===================== 通用小工具 ===================== */

lv_obj_t * ui_plain_obj(lv_obj_t * parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

lv_obj_t * ui_gradient_bg(lv_obj_t * parent, lv_coord_t w, lv_coord_t h,
                          lv_color_t top, lv_color_t bottom, lv_coord_t radius)
{
    lv_obj_t * obj = ui_plain_obj(parent, w, h);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(obj, top, 0);
    lv_obj_set_style_bg_grad_color(obj, bottom, 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    return obj;
}

lv_obj_t * ui_label(lv_obj_t * parent, const lv_font_t * font, lv_color_t color, const char * text)
{
    lv_obj_t * label = lv_label_create(parent);
    if(font) lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, text);
    return label;
}

lv_obj_t * ui_pill(lv_obj_t * parent, lv_coord_t w, lv_coord_t h,
                   lv_color_t color, lv_opa_t opa)
{
    lv_obj_t * obj = ui_plain_obj(parent, w, h);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
    return obj;
}

lv_obj_t * ui_img(lv_obj_t * parent, const char * asset_rel)
{
    lv_obj_t * img = lv_img_create(parent);
    const lv_img_dsc_t * dsc = assets_png(asset_rel);
    if(dsc) {
        lv_img_set_src(img, dsc);
    }
    else {
        /* 资源缺失时不崩，显示一个占位方块 */
        lv_obj_set_size(img, UI_ICON_IMG_SIZE, UI_ICON_IMG_SIZE);
        lv_obj_set_style_bg_color(img, lv_color_hex(0x888888), 0);
        lv_obj_set_style_bg_opa(img, LV_OPA_COVER, 0);
    }
    return img;
}
