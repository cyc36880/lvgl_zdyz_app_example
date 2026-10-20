/**
 * @file snapshot.c
 */
#include "myCode/Common/snapshot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char s_dir[512];

void snapshot_init(void)
{
    const char * env = getenv("MYGUI_SNAPSHOT_DIR");
    snprintf(s_dir, sizeof(s_dir), "%s", (env && env[0]) ? env : "");
}

bool snapshot_enabled(void)
{
    return s_dir[0] != '\0';
}

const char * snapshot_dir(void)
{
    return s_dir;
}

static void put_u16(uint8_t * p, uint16_t v) { p[0] = (uint8_t)(v & 0xff); p[1] = (uint8_t)(v >> 8); }
static void put_u32(uint8_t * p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
    p[2] = (uint8_t)((v >> 16) & 0xff);
    p[3] = (uint8_t)((v >> 24) & 0xff);
}

bool snapshot_save(const char * name)
{
    if(!snapshot_enabled()) return false;

    lv_img_dsc_t * shot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR_ALPHA);
    if(shot == NULL) {
        LV_LOG_WARN("snapshot failed");
        return false;
    }

    uint32_t w = shot->header.w;
    uint32_t h = shot->header.h;
    uint32_t row_bytes = w * 4;
    uint32_t img_bytes = row_bytes * h;

    char path[700];
    snprintf(path, sizeof(path), "%s/%s.bmp", s_dir, name);

    FILE * f = fopen(path, "wb");
    if(f == NULL) {
        LV_LOG_WARN("cannot write %s", path);
        lv_snapshot_free(shot);
        return false;
    }

    uint8_t header[54];
    memset(header, 0, sizeof(header));
    header[0] = 'B';
    header[1] = 'M';
    put_u32(&header[2], 54 + img_bytes);
    put_u32(&header[10], 54);
    put_u32(&header[14], 40);
    put_u32(&header[18], w);
    put_u32(&header[22], h);
    put_u16(&header[26], 1);
    put_u16(&header[28], 32);
    put_u32(&header[30], 0);          /* BI_RGB */
    put_u32(&header[34], img_bytes);
    fwrite(header, 1, sizeof(header), f);

    /* LV_COLOR_DEPTH == 32 时像素就是 BGRA，BMP 的 32bpp 也是 BGRA，可以直接写。
     * 唯一的区别是 BMP 的行顺序是自下而上。 */
    const uint8_t * data = (const uint8_t *)shot->data;
    for(int32_t y = (int32_t)h - 1; y >= 0; y--) {
        fwrite(data + (size_t)y * row_bytes, 1, row_bytes, f);
    }

    fclose(f);
    lv_snapshot_free(shot);
    LV_LOG_USER("snapshot: %s", path);
    return true;
}
