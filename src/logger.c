/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

#define LOG_CONTENT_MAX_LEN  2048

static string_t level_str[] = {
    string_null,
    xstring(" [ERROR] "),
    xstring(" [WARN] "),
    xstring(" [INFO] "),
    xstring(" [DEBUG] "),
    xstring(" [STDOUT] ")
};

static void log_core_writer(file_t *f, int err, int level, const char *fmt, 
        va_list args)
{
    int     n;
    u_char *p, *str_err;
    u_char  info_str[LOG_CONTENT_MAX_LEN];
    size_t  max_len = 0;

    p = info_str;

    p = x_memcpy_n(p, cache_log_time.data, cache_log_time.len);
    
    p = x_memcpy_n(p, level_str[level].data, level_str[level].len);  
    
    if (err != 0) {
        str_err = (u_char *) strerror(err);
        p = x_memcpy_n(p, str_err, x_strlen(str_err)); 
        p = x_memcpy_n(p, " - ", 3);
    }

    max_len = LOG_CONTENT_MAX_LEN - level_str[level].len - 1; /* -1 for '\n'*/

    n = vsnprintf((char *) p, max_len, fmt, args);
    p += (n < max_len ? n : max_len - 1);

    *(p++) = LF;

    if (f == NULL) {
        (void) write_stdout(info_str, p - info_str);
    } else {
        file_write(f, info_str, p - info_str, FL_DEFAULT_OFFSET);
    }
}

logger_t *log_create(mem_pool_t *pool, int level, file_t *file)
{
    logger_t *logger;

    logger = pmalloc(pool, sizeof(logger_t));
    if (logger == NULL) {
        log_error(pool->logger, 0, "pmalloc failed");
        return NULL;
    }

    logger->level = level;
    logger->file = file;

    return logger;
}

void log_error(logger_t *logger, int err, const char *fmt, ...)
{
    va_list args;

    if (logger == NULL) {
        return;
    }

    va_start(args, fmt);
    log_core_writer(logger->file, err, LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

void log_warn(logger_t  *logger, int err, const char *fmt, ...)
{
    va_list args;

    if (logger == NULL) {
        return;
    }

    if (logger->level < LOG_LEVEL_WARN) {
        return;
    }

    va_start(args, fmt);
    log_core_writer(logger->file, err, LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void log_info(logger_t *logger, int err, const char *fmt, ...)
{
    va_list args;

    if (logger == NULL) {
        return;
    }

    if (logger->level < LOG_LEVEL_INFO) {
        return;
    }

    va_start(args, fmt);
    log_core_writer(logger->file, err, LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void log_debug(logger_t *logger, int err, const char *fmt, ...)
{
    va_list args;

    if (logger == NULL) {
        return;
    }

    if (logger->level < LOG_LEVEL_DEBUG) {
        return;
    }

    va_start(args, fmt);
    log_core_writer(logger->file, err, LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void log_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    log_core_writer(NULL, 0, LOG_LEVEL_STDOUT, fmt, args);
    va_end(args);
}

void log_stderr(int err, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    log_core_writer(NULL, err, LOG_LEVEL_STDOUT, fmt, args);
    va_end(args);
}
