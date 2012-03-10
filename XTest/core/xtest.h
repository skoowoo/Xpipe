/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __XTEST_H__
#define __XTEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define TEST_ERROR -1
#define TEST_OK 0

#define test_abort() return TEST_ERROR

void test_case_name(const char *name);
void test_ok(const char *fmt, ...);
void test_fail(const char *fmt, ...);

typedef int (*prepare_test_handler_fp) (void);
typedef int (*run_test_handler_fp) (void);
typedef int (*finish_test_handler_fp) (void);


typedef struct {
    char  *name;

    prepare_test_handler_fp  prepare;
    run_test_handler_fp      run;
    finish_test_handler_fp   finish;
} unit_cases_t;


#define TEST_CASE(name)  test_case_name(name);

#define ASSERT_NOT_NULL(p)  \
    if ((p) != NULL) {       \
        test_ok("pointer is not \"NULL\"\n");                    \
    } else {                 \
        test_fail("pointer is \"NULL\"\n");                     \
        test_abort(); \
    }


#define ASSERT_EQ(n1, n2)   \
    if ((n1) == (n2)) {     \
        test_ok("%d == %d\n", (n1), (n2));\
    } else {                \
        test_fail("%d == %d\n", (n1), (n2));\
        test_abort();\
    }

#define ASSERT_NE(n1, n2)                           \
    if ((n1) != (n2)) {                             \
        test_ok("%d != %d\n", (n1), (n2));\
    } else {                                        \
        test_fail("%d != %d\n", (n1), (n2));\
        test_abort();\
    }                                               \

#define ASSERT_GT(n1, n2)                           \
    if ((n1) > (n2)) {                              \
        test_ok("%d > %d\n", (n1), (n2)); \
    } else {                                        \
        test_fail("%d > %d\n", (n1), (n2));\
        test_abort(); \
    } 

#define ASSERT_GE(n1, n2)                           \
    if ((n1) >= (n2)) {                             \
        test_ok("%d >= %d\n", (n1), (n2));\
    } else {                                        \
        test_fail("%d >= %d\n", (n1), (n2));\
        test_abort(); \
    }

#define ASSERT_LT(n1, n2)                           \
    if ((n1) < (n2)) {                              \
        test_ok("%d < %d\n", (n1), (n2));\
    } else {                                        \
        test_fail("%d < %d\n", (n1), (n2));\
        test_abort(); \
    }

#define ASSERT_LE(n1, n2)                           \
    if ((n1) <= (n2)) {                             \
        test_ok("%d <= %d\n", (n1), (n2));\
    } else {                                        \
        test_fail("%d <= %d\n", (n1), (n2));\
        test_abort(); \
    }

#define ASSERT_STR_EQ(s1, s2)                       \
    if (strcmp((s1), (s2)) == 0) {                  \
        test_ok("%s == %s\n", (s1), (s2));\
    } else {                                        \
        test_fail("%s == %s\n", (s1), (s2));\
        test_abort();\
    }

#define ASSERT_STR_GT(s1, s2)                       \
    if (strcmp((s1), (s2)) > 0) {                   \
        test_ok("%s > %s", (s1), (s2));\
    } else {                                        \
        test_fail("%s > %s", (s1), (s2));\
        test_abort(); \
    }

#define ASSERT_STR_GE(s1, s2)                       \
    if (strcmp((s1), (s2)) >= 0) {                  \
        test_ok("%s >= %s\n", (s1), (s2));\
    } else {                                        \
        test_fail("%s >= %s\n", (s1), (s2));\
        test_abort(); \
    }

#define ASSERT_STR_LT(s1, s2)                       \
    if (strcmp((s1), (s2)) < 0) {                   \
        test_ok("%s < %s\n", (s1), (s2));\
    } else {                                        \
        test_fail("%s < %s\n", (s1), (s2));\
        test_abort(); \
    }

#define ASSERT_STR_LE(s1, s2)                       \
    if (strcmp((s1), (s2)) <= 0) {                  \
        test_ok("%s <= %s\n", (s1), (s2));\
    } else {                                        \
        test_fail("%s <= %s\n", (s1), (s2));\
        test_abort(); \
    }

#endif /* __XTEST_H__ */
