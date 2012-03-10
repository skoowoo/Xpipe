/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

int is_positive_integer(string_t *num)
{
    uint_t i;

    if (num == NULL || num->len == 0) {
        return 0;
    }

    if (!is_digit(*(num->data)) || *(num->data) == '0') {
        return 0;
    }

    for (i = 1; i < num->len; i++) {
        if (!is_digit(*(num->data + i))) {
            return 0;
        }
    }

    return 1;
}

void make_full_file_name(u_char *full_name, u_char *name, int len)
{
    u_char *p;

    p = full_name;

    p = x_memcpy_n(p, xpipe_install_dir_path, strlen(xpipe_install_dir_path));

    if (*(xpipe_install_dir_path + strlen(xpipe_install_dir_path) - 1) != '/')
    {
        p = x_memcpy_n(p, "/", 1);
    }

    p = x_memcpy_n(p, name, len);
}
