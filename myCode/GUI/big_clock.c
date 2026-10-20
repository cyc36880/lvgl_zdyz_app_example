/**
 * @file big_clock.c
 */
#include "myCode/GUI/big_clock.h"
#include "myCode/GUI/widgets.h"
#include "myCode/Common/assets.h"
#include "myCode/Common/theme.h"

#include <stdio.h>

struct big_clock {
    lv_obj_t * cont;
    lv_obj_t * digits[5];   /* 时十位、时个位、冒号、分十位、分个位 */
    lv_color_t color;
};

/* 11 张位图的描述符缓存：0~9 然后是冒号 */
static const lv_img_dsc_t * s_digit_dsc[11];

static const lv_img_dsc_t * digit_dsc(int idx)
{
    if(s_digit_dsc[idx] == NULL) {
        char name[64];
        if(idx < 10) {
            /* 对应 CanMV 的 "A:" + RESOURCES_PATH + "Fonts/..."，这里资源根目录就是 myCode/ */
            snprintf(name, sizeof(name), "Fonts/clock_digit_%d.png", idx);
        }
        else {
            snprintf(name, sizeof(name), "Fonts/clock_colon.png");
        }
        s_digit_dsc[idx] = assets_png(name);
    }
    return s_digit_dsc[idx];
}

static void apply_recolor(big_clock_t * clock, lv_obj_t * img)
{
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
    lv_obj_set_style_img_recolor(img, clock->color, 0);
}

static void set_digit(big_clock_t * clock, int slot, int idx)
{
    const lv_img_dsc_t * dsc = digit_dsc(idx);
    if(dsc == NULL) return;
    lv_img_set_src(clock->digits[slot], dsc);
    apply_recolor(clock, clock->digits[slot]);
}

big_clock_t * big_clock_create(lv_obj_t * parent)
{
    big_clock_t * clock = (big_clock_t *)lv_mem_alloc(sizeof(big_clock_t));
    lv_memset_00(clock, sizeof(big_clock_t));
    clock->color = UI_COLOR_TEXT_DARK;

    clock->cont = ui_plain_obj(parent, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(clock->cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(clock->cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(clock->cont, 4, 0);

    for(int i = 0; i < 5; i++) {
        clock->digits[i] = lv_img_create(clock->cont);
    }

    big_clock_set_time(clock, 0, 0);
    return clock;
}

lv_obj_t * big_clock_obj(big_clock_t * clock)
{
    return clock ? clock->cont : NULL;
}

void big_clock_set_time(big_clock_t * clock, int hour, int minute)
{
    if(clock == NULL) return;

    hour   = hour   % 24; if(hour   < 0) hour   += 24;
    minute = minute % 60; if(minute < 0) minute += 60;

    set_digit(clock, 0, (hour / 10) % 10);
    set_digit(clock, 1, hour % 10);
    set_digit(clock, 2, 10);            /* 冒号 */
    set_digit(clock, 3, (minute / 10) % 10);
    set_digit(clock, 4, minute % 10);
}

void big_clock_set_color(big_clock_t * clock, lv_color_t color)
{
    if(clock == NULL) return;
    clock->color = color;
    for(int i = 0; i < 5; i++) {
        apply_recolor(clock, clock->digits[i]);
    }
}
