/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "config.h"
#include "system.h"

#define LOG_LEVEL_ERROR     1
#define LOG_LEVEL_WARN      2
#define LOG_LEVEL_INFO      3
#define LOG_LEVEL_DEBUG     4
#define LOG_LEVEL_STDOUT    5

#define LOG_FILE_STDOUT NULL

struct logger_s {
    int      level;
    file_t  *file;
};


logger_t* log_create(mem_pool_t *pool, int level, file_t *file);

void log_error(logger_t *logger, int err, const char *fmt, ...);
void log_warn(logger_t  *logger, int err, const char *fmt, ...);
void log_info(logger_t *logger, int err, const char *fmt, ...);
void log_debug(logger_t *logger, int err, const char *fmt, ...);

void log_printf(const char *fmt, ...);
void log_stderr(int err, const char *fmt, ...);

#endif  /* __LOGGER_H__ */
