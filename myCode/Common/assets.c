/**
 * @file assets.c
 * PNG 资源的读盘与缓存。
 */
#include "myCode/Common/assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 由 CMakeLists.txt 传入的绝对路径，例如 D:/zishen/.../myCode */
#ifndef MYGUI_ASSET_DIR
#define MYGUI_ASSET_DIR "myCode"
#endif

#define ASSET_CACHE_MAX 64
#define ASSET_NAME_MAX  80

typedef struct {
    char                name[ASSET_NAME_MAX];
    lv_img_dsc_t *      dsc;
} asset_cache_entry_t;

static asset_cache_entry_t s_cache[ASSET_CACHE_MAX];
static int                 s_cache_cnt;
static char                s_root[512];

void assets_init(void)
{
    const char * env = getenv("MYGUI_ASSETS");
    snprintf(s_root, sizeof(s_root), "%s", (env && env[0]) ? env : MYGUI_ASSET_DIR);

    /* 统一去掉结尾的斜杠，拼路径时就不会出现 "//" */
    size_t n = strlen(s_root);
    while(n > 1 && (s_root[n - 1] == '/' || s_root[n - 1] == '\\')) {
        s_root[--n] = '\0';
    }
    LV_LOG_USER("asset root: %s", s_root);
}

const char * assets_root(void)
{
    return s_root;
}

/** 把整个文件读进一块用 lv_mem_alloc() 申请的内存里（生命周期 = 程序运行期）。 */
static uint8_t * read_file(const char * path, size_t * out_len)
{
    FILE * f = fopen(path, "rb");
    if(f == NULL) return NULL;

    if(fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    if(len <= 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    uint8_t * buf = (uint8_t *)lv_mem_alloc((uint32_t)len);
    if(buf == NULL) {
        fclose(f);
        return NULL;
    }
    size_t rd = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if(rd != (size_t)len) {
        lv_mem_free(buf);
        return NULL;
    }
    *out_len = (size_t)len;
    return buf;
}

/**
 * 从 PNG 数据里读出宽高（IHDR 块的前 8 个字节）。
 *
 * 这一步是必须的：LVGL 的 lv_img_decoder_get_info() 会先问"内置解码器"，
 * 而内置解码器对 LV_IMG_SRC_VARIABLE 直接把 dsc->header.w/h 原样返回。
 * 如果留 0，lv_img 就会被摆成 0x0 的尺寸，什么都看不见。
 * 把宽高填对之后，内置解码器返回正确的尺寸，真正画的时候才交给 PNG 解码器。
 */
static bool png_read_size(const uint8_t * data, size_t len, uint32_t * w, uint32_t * h)
{
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a };
    if(len < 24 || memcmp(data, sig, 8) != 0) return false;

    *w = ((uint32_t)data[16] << 24) | ((uint32_t)data[17] << 16) |
         ((uint32_t)data[18] << 8) | (uint32_t)data[19];
    *h = ((uint32_t)data[20] << 24) | ((uint32_t)data[21] << 16) |
         ((uint32_t)data[22] << 8) | (uint32_t)data[23];
    return (*w > 0 && *h > 0);
}

const lv_img_dsc_t * assets_png(const char * rel)
{
    if(rel == NULL) return NULL;

    for(int i = 0; i < s_cache_cnt; i++) {
        if(strcmp(s_cache[i].name, rel) == 0) return s_cache[i].dsc;
    }

    if(s_cache_cnt >= ASSET_CACHE_MAX) {
        LV_LOG_WARN("asset cache full, '%s' not cached", rel);
        return NULL;
    }

    char path[640];
    snprintf(path, sizeof(path), "%s/%s", s_root, rel);

    size_t len = 0;
    uint8_t * data = read_file(path, &len);
    if(data == NULL) {
        LV_LOG_WARN("cannot read image: %s", path);
        return NULL;
    }

    uint32_t w = 0, h = 0;
    if(!png_read_size(data, len, &w, &h)) {
        LV_LOG_WARN("not a valid PNG: %s", path);
        lv_mem_free(data);
        return NULL;
    }

    lv_img_dsc_t * dsc = (lv_img_dsc_t *)lv_mem_alloc(sizeof(lv_img_dsc_t));
    if(dsc == NULL) {
        lv_mem_free(data);
        return NULL;
    }

    /* cf = LV_IMG_CF_RAW_ALPHA 表示"这是一份原始编码数据（PNG），交给解码器处理"。
     * 宽高必须填，否则图片尺寸是 0（见上面 png_read_size 的注释）。 */
    lv_memset_00(dsc, sizeof(lv_img_dsc_t));
    dsc->header.always_zero = 0;
    dsc->header.cf          = LV_IMG_CF_RAW_ALPHA;
    dsc->header.w           = (uint32_t)w;
    dsc->header.h           = (uint32_t)h;
    dsc->data               = data;
    dsc->data_size          = (uint32_t)len;

    snprintf(s_cache[s_cache_cnt].name, ASSET_NAME_MAX, "%s", rel);
    s_cache[s_cache_cnt].dsc = dsc;
    s_cache_cnt++;

    return dsc;
}
