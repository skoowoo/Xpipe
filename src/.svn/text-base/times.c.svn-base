/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

volatile uint_t time_current_msecs;
volatile time_t time_current_seconds;

volatile string_t cache_log_time; 

static u_char log_time[21];

void timer_init()
{
    memset(log_time, 0, 21);

    cache_log_time.data = log_time;
    cache_log_time.len = 20;

    timer_update();
}

void timer_update(void)
{
    time_t          secs;
    uint_t          msecs;
    struct tm       tm, *tmp;
    struct timeval  tv;

    gettimeofday(&tv, NULL);

    secs = tv.tv_sec;
    msecs = tv.tv_usec / 1000;

    time_current_seconds = secs;
    time_current_msecs = (uint_t) secs * 1000 + msecs;

    tmp = localtime(&secs);
    tm = *tmp;

    tm.tm_mon++;
    tm.tm_year += 1900;

    sprintf((char *) log_time, "%4d-%02d-%02d/%02d:%02d:%02d ",
            tm.tm_year, tm.tm_mon, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
}
