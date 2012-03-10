/**
 * Copyright (c) Xiaowei Wu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xtest.h"

#ifdef NEW_CONFIG
#include "../config.h"
#else
#include "config.h"
#endif

#define c_close     "\e[0m"
#define c_red       "\e[1;31m"
#define c_green     "\e[1;32m"
#define c_yellow    "\e[1;33m" 

#define set_red(text)       "\e[1;31m"text"\e[0m"
#define set_green(text)     "\e[1;32m"text"\e[0m"
#define set_yellow(text)    "\e[1;33m"text"\e[0m"

typedef struct {
    
} cases_summary_t;


int main(int argc, char *argv[])
{
    int           i, ret;
    unit_cases_t *tc;
    
    for (i = 0; ; i++) {
        tc = test_units[i];
        if (tc == NULL) {
            break;
        }

        ret = tc->prepare();
        if (ret == TEST_ERROR) {
            printf("%s%s prepare failed.%s\n", c_red, tc->name, c_close);
            continue;
        }

        ret = tc->run();
        if (ret == TEST_ERROR) {
            printf("%s%s run failed.%s\n", c_red, tc->name, c_close);
        }

        ret = tc->finish();
        if (ret == TEST_ERROR) {
            printf("%s%s finish failed.%s\n", c_red, tc->name, c_close);
        }
    } 

    return 0;
}

void test_case_name(const char *name)
{
    printf("%s================= %s ===============%s\n", 
            c_yellow, name, c_close);
}

void test_ok(const char *fmt, ...)
{
    va_list args;

    printf(set_green("[PASS] "));

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void test_fail(const char *fmt, ...)
{
    va_list args;

    printf(set_red("[FAIL] "));

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
