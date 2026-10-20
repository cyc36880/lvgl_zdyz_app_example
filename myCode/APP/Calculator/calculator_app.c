/**
 * @file calculator_app.c
 *
 * 原 main.py 的 AppCalculator 里，按钮排布是写死的一维列表（20 个），
 * 用 lv.grid 做 4 行 x 5 列；算式的合法性判断则是一大串 if/elif。
 *
 * 这里保留同样的外观（黑色背景 + 深紫数字键 + 灰功能键 + 橙色运算符），
 * 但把"算式"重写成一个小状态机 + 一个简单的解析器，逻辑更好读：
 *   - expr[] 里只存内部形式，例如 "12+3.5"，运算符为 + - * /
 *   - 显示时再把 * 换成 ×、/ 换成 ÷
 */
#include "myCode/APP/Calculator/calculator_app.h"
#include "myCode/Common/theme.h"
#include "myCode/GUI/widgets.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BTN_ROWS 4
#define BTN_COLS 5

typedef struct {
    char        expr[48];     /* 内部算式："12+3.5" */
    bool        pointed;      /* 当前数字已经有点了 */
    bool        calc_done;    /* 上一步按的是运算符 */
    bool        got_result;   /* 上一步按的是 = */
    bool        error;        /* 除零错误 */
    lv_obj_t *  show;         /* 大号显示 */
    lv_obj_t *  history;      /* 灰色历史 */
} calc_t;

/* 按钮文字与颜色：顺序和原代码一一对应（行优先）。
 * 注意：原代码用的是 "×" 和 "÷"，但 PC 版内置的 Montserrat 字体只覆盖 ASCII，
 * 没有 U+00D7 / U+00F7 的字形（会显示成空白方块），所以这里用 ASCII 的 * 和 /。 */
static const char * const s_btn_label[BTN_ROWS * BTN_COLS] = {
    "7",  "8", "9", "AC",  "/",
    "4",  "5", "6", "+/-", "*",
    "1",  "2", "3", "%",   "-",
    "",   "0", ".", "=",   "+",
};

static const uint32_t s_btn_color[BTN_ROWS * BTN_COLS] = {
    0x211924, 0x211924, 0x211924, 0x584B5E, 0xF99429,
    0x211924, 0x211924, 0x211924, 0x584B5E, 0xF99429,
    0x211924, 0x211924, 0x211924, 0x584B5E, 0xF99429,
    0x211924, 0x211924, 0x211924, 0xF99429, 0xF99429,
};

/* ---------------- 显示 ---------------- */

static void calc_display(calc_t * c)
{
    lv_label_set_text(c->show, (c->expr[0] != '\0') ? c->expr : "0");
}

/* ---------------- 输入 ---------------- */

static bool is_op(char ch)
{
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

static void calc_reset(calc_t * c, bool clear_history)
{
    c->expr[0]   = '\0';
    c->pointed   = false;
    c->calc_done = false;
    c->got_result = false;
    c->error     = false;
    if(clear_history) lv_label_set_text(c->history, "");
}

/** 如果上一步是"= 出结果"或者出错，先清空再输入新内容。 */
static void prepare_input(calc_t * c)
{
    if(c->error) {
        calc_reset(c, true);
    }
    else if(c->got_result) {
        calc_reset(c, true);
    }
}

static void calc_digit(calc_t * c, char d)
{
    prepare_input(c);

    size_t n = strlen(c->expr);
    if(strcmp(c->expr, "0") == 0) {
        c->expr[0] = '\0';
        n = 0;
    }
    if(c->calc_done) {          /* 运算符后面开始一个新数字 */
        c->calc_done = false;
        c->pointed   = false;
        n = strlen(c->expr);
    }
    if(n < sizeof(c->expr) - 2) {
        c->expr[n]     = d;
        c->expr[n + 1] = '\0';
    }
    calc_display(c);
}

static void calc_op(calc_t * c, char op)
{
    if(c->error) calc_reset(c, true);
    if(c->got_result) c->got_result = false;   /* 用结果继续算 */

    size_t n = strlen(c->expr);
    if(n == 0) {
        if(op == '-') {                        /* 允许负数开头 */
            c->expr[0] = '-';
            c->expr[1] = '\0';
            c->calc_done = false;
        }
        else {
            c->expr[0] = '0';
            c->expr[1] = op;
            c->expr[2] = '\0';
            c->calc_done = true;
        }
        c->pointed = false;
        calc_display(c);
        return;
    }

    char last = c->expr[n - 1];
    if(is_op(last)) {
        c->expr[n - 1] = op;                   /* 连按运算符 → 直接替换 */
    }
    else if(last == '.') {
        c->expr[n - 1] = op;                   /* "3." 后面按运算符 → 去掉点 */
    }
    else {
        c->expr[n]     = op;
        c->expr[n + 1] = '\0';
    }
    c->calc_done = true;
    c->pointed   = false;
    calc_display(c);
}

static void calc_point(calc_t * c)
{
    prepare_input(c);

    if(c->calc_done) {
        c->calc_done = false;
        c->pointed   = false;
    }
    if(c->pointed) return;

    size_t n = strlen(c->expr);
    if(n == 0 || is_op(c->expr[n - 1])) {
        c->expr[n]     = '0';
        c->expr[n + 1] = '\0';
        n = strlen(c->expr);
    }
    if(n < sizeof(c->expr) - 2) {
        c->expr[n]     = '.';
        c->expr[n + 1] = '\0';
    }
    c->pointed = true;
    calc_display(c);
}

/** +/-：整体取反（够用且不会让人猜不透） */
static void calc_negate(calc_t * c)
{
    prepare_input(c);
    if(c->expr[0] == '\0') return;

    if(c->expr[0] == '-') {
        memmove(c->expr, c->expr + 1, strlen(c->expr));
    }
    else if(strlen(c->expr) < sizeof(c->expr) - 2) {
        memmove(c->expr + 1, c->expr, strlen(c->expr) + 1);
        c->expr[0] = '-';
    }
    calc_display(c);
}

/** %：把当前这个数字除以 100 */
static void calc_percent(calc_t * c)
{
    if(c->error || c->expr[0] == '\0') return;

    size_t n = strlen(c->expr);
    size_t start = (c->expr[0] == '-') ? 1 : 0;
    for(size_t i = start; i < n; i++) {
        if(is_op(c->expr[i])) start = i + 1;
    }
    if(start >= n) return;

    double v = atof(c->expr + start) / 100.0;
    char num[24];
    snprintf(num, sizeof(num), "%.6g", v);
    snprintf(c->expr + start, sizeof(c->expr) - start, "%s", num);
    calc_display(c);
}

static void calc_equal(calc_t * c)
{
    size_t n = strlen(c->expr);
    if(n == 0 || c->error) return;

    /* 去掉结尾多余的运算符 / 小数点 */
    while(n > 0 && (is_op(c->expr[n - 1]) || c->expr[n - 1] == '.')) {
        c->expr[--n] = '\0';
    }
    if(n == 0) return;

    /* 历史区先记下这次的算式 */
    lv_label_set_text(c->history, c->expr);

    /* 找到第一个运算符（允许开头是负号） */
    int idx = -1;
    for(size_t i = (c->expr[0] == '-') ? 1u : 0u; i < n; i++) {
        if(is_op(c->expr[i])) {
            idx = (int)i;
            break;
        }
    }
    if(idx < 0) {                 /* 没有运算符，就是一个数 */
        c->got_result = true;
        return;
    }

    char op = c->expr[idx];
    char left[32];
    memcpy(left, c->expr, (size_t)idx);
    left[idx] = '\0';
    const char * right = c->expr + idx + 1;

    double x = atof(left);
    double y = atof(right);
    double r = 0;
    bool   ok = true;

    switch(op) {
        case '+': r = x + y; break;
        case '-': r = x - y; break;
        case '*': r = x * y; break;
        case '/':
            if(y == 0) ok = false;
            else r = x / y;
            break;
        default: ok = false; break;
    }

    if(!ok) {
        snprintf(c->expr, sizeof(c->expr), "Error");
        c->error = true;
    }
    else {
        snprintf(c->expr, sizeof(c->expr), "%.6g", r);
    }
    c->got_result = true;
    c->pointed    = false;
    c->calc_done  = false;
    calc_display(c);
}

/* ---------------- 事件 ---------------- */

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * handle    = lv_event_get_target(e);
    calc_t *   c         = (calc_t *)lv_event_get_user_data(e);
    lv_obj_t * btn       = lv_obj_get_parent(handle);
    lv_obj_t * label     = lv_obj_get_child(btn, 0);
    const char * text    = lv_label_get_text(label);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_set_style_bg_opa(handle, 100, 0);     /* 白色半透明高亮 */
        return;
    }
    if(code == LV_EVENT_RELEASED) {
        lv_obj_set_style_bg_opa(handle, LV_OPA_TRANSP, 0);
        return;
    }
    if(code != LV_EVENT_CLICKED || text[0] == '\0') return;

    /* "AC" */
    if(strcmp(text, "AC") == 0) {
        calc_reset(c, true);
        calc_display(c);
        return;
    }
    /* "=" */
    if(strcmp(text, "=") == 0) {
        calc_equal(c);
        return;
    }
    /* "+/-" */
    if(strcmp(text, "+/-") == 0) {
        calc_negate(c);
        return;
    }
    /* "%" */
    if(strcmp(text, "%") == 0) {
        calc_percent(c);
        return;
    }
    /* "." */
    if(strcmp(text, ".") == 0) {
        calc_point(c);
        return;
    }
    /* "*" "/" */
    if(text[0] == '*') {
        calc_op(c, '*');
        return;
    }
    if(text[0] == '/') {
        calc_op(c, '/');
        return;
    }
    if(text[0] == '+' || text[0] == '-') {
        calc_op(c, text[0]);
        return;
    }
    if(isdigit((unsigned char)text[0])) {
        calc_digit(c, text[0]);
    }
}

/* ---------------- 创建 ---------------- */

lv_obj_t * calculator_app_create(lv_obj_t * parent, void * user_ctx)
{
    (void)user_ctx;

    calc_t * c = (calc_t *)lv_mem_alloc(sizeof(calc_t));
    lv_memset_00(c, sizeof(calc_t));

    lv_obj_t * page = ui_plain_obj(parent, lv_pct(100), lv_pct(100));
    lv_obj_align(page, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);

    /* ---- 显示区 ---- */
    lv_obj_t * show_conv = ui_plain_obj(page, lv_pct(95), lv_pct(25));
    lv_obj_align(show_conv, LV_ALIGN_TOP_MID, 0, 40);

    c->history = ui_label(show_conv, UI_FONT_NORMAL, lv_color_hex(0x7F7F7F), "");
    lv_obj_align(c->history, LV_ALIGN_BOTTOM_RIGHT, -10, -80);

    c->show = ui_label(show_conv, UI_FONT_HUGE, lv_color_hex(0xFFFFFF), "0");
    lv_obj_align(c->show, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    /* ---- 键盘：5 列 x 4 行的 grid ---- */
    lv_obj_t * keyboard = ui_plain_obj(page, lv_pct(95), lv_pct(60));
    lv_obj_align_to(keyboard, show_conv, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    static lv_coord_t col_dsc[BTN_COLS + 1];
    static lv_coord_t row_dsc[BTN_ROWS + 1];
    for(int i = 0; i < BTN_COLS; i++) col_dsc[i] = LV_GRID_FR(1);
    col_dsc[BTN_COLS] = LV_GRID_TEMPLATE_LAST;
    for(int i = 0; i < BTN_ROWS; i++) row_dsc[i] = LV_GRID_FR(1);
    row_dsc[BTN_ROWS] = LV_GRID_TEMPLATE_LAST;

    lv_obj_set_grid_dsc_array(keyboard, col_dsc, row_dsc);
    lv_obj_set_layout(keyboard, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_column(keyboard, 10, 0);
    lv_obj_set_style_pad_row(keyboard, 10, 0);

    for(int row = 0; row < BTN_ROWS; row++) {
        for(int col = 0; col < BTN_COLS; col++) {
            int idx = row * BTN_COLS + col;

            lv_obj_t * btn = lv_obj_create(keyboard);
            lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1,
                                 LV_GRID_ALIGN_STRETCH, row, 1);
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_radius(btn, 50, 0);
            lv_obj_set_style_clip_corner(btn, true, 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(s_btn_color[idx]), 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);   /* 由内部 handle 收事件 */

            lv_obj_t * label = ui_label(btn, UI_FONT_TITLE, lv_color_hex(0xFFFFFF), s_btn_label[idx]);
            lv_obj_center(label);

            /* 覆盖整格的透明"手柄"：按下变白 100/255，松开变透明 —— 原代码的按压缩放效果 */
            lv_obj_t * handle = ui_plain_obj(btn, lv_pct(100), lv_pct(100));
            lv_obj_center(handle);
            lv_obj_set_style_radius(handle, 0, 0);
            lv_obj_set_style_bg_color(handle, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(handle, LV_OPA_TRANSP, 0);
            lv_obj_add_flag(handle, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(handle, btn_event_cb, LV_EVENT_PRESSED, c);
            lv_obj_add_event_cb(handle, btn_event_cb, LV_EVENT_RELEASED, c);
            lv_obj_add_event_cb(handle, btn_event_cb, LV_EVENT_CLICKED, c);
        }
    }

    calc_display(c);
    return page;
}
