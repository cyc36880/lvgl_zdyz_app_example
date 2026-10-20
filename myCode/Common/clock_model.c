/**
 * @file clock_model.c
 */
#include "myCode/Common/clock_model.h"

/* 相对真实时间的偏移（秒）。0 表示和电脑时间一致。 */
static long s_offset = 0;

static time_t now_with_offset(void)
{
    return time(NULL) + s_offset;
}

static void local_time_of(time_t t, struct tm * out)
{
#ifdef _WIN32
    localtime_s(out, &t);
#else
    *out = *localtime(&t);
#endif
}

void clock_model_init(void)
{
    s_offset = 0;
}

void clock_model_get(struct tm * out)
{
    local_time_of(now_with_offset(), out);
}

void clock_model_set(int year, int mon, int day, int hour, int min, int sec)
{
    struct tm target;
    target.tm_year  = year - 1900;
    target.tm_mon   = mon - 1;
    target.tm_mday  = day;
    target.tm_hour  = hour;
    target.tm_min   = min;
    target.tm_sec   = sec;
    target.tm_isdst = -1;
    target.tm_wday  = 0;
    target.tm_yday  = 0;

    time_t want = mktime(&target);
    if(want == (time_t)-1) return;

    s_offset = (long)(want - time(NULL));
}

void clock_model_reset(void)
{
    s_offset = 0;
}
