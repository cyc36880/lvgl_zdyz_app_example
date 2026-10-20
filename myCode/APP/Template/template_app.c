/**
 * @file template_app.c
 *
 * 对应原 main.py 里的：
 *     class AppTemplate(AppBase):
 *         def __init__(...):
 *             super().__init__(...)
 *             self.btn = lv.btn(self.conv)
 *             self.btn.set_size(200, 100)
 *             self.btn.center()
 *
 * 在 C 版里，"App" 就是"一个往 parent 里画界面的函数"，
 * 生命周期（masker 缩放、home bar、拖拽关闭）都由 GUI/app_manager.c 统一处理。
 */
#include "myCode/APP/Template/template_app.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

lv_obj_t * template_app_create(lv_obj_t * parent, void * user_ctx)
{
    (void)user_ctx;

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, UI_COLOR_BG_TOP, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    lv_obj_t * title = ui_label(page, UI_FONT_HUGE, UI_COLOR_TEXT_DARK, "Template");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 90);

    lv_obj_t * btn = lv_btn_create(page);
    lv_obj_set_size(btn, 240, 90);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_radius(btn, 24, 0);
    lv_obj_set_style_bg_color(btn, UI_COLOR_ACCENT, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 0, 0);

    lv_obj_t * btn_label = ui_label(btn, UI_FONT_TITLE, UI_COLOR_TEXT_LIGHT, "Hello");
    lv_obj_center(btn_label);

    return page;
}
