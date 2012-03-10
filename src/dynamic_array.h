/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "config.h"
#include "system.h"

struct dynamic_array_s {
    void        *elts;
    size_t       size;
    uint_t       nelts;
    uint_t       nalloc;
    mem_pool_t  *pool;
};


dynamic_array_t *dynamic_array_create(mem_pool_t *pool, uint_t n, uint_t size);
void dynamic_array_destroy(dynamic_array_t *array);
void *dynamic_array_push(dynamic_array_t *array);
void *dynamic_array_push_ix(dynamic_array_t *array, uint_t index);
void *dynamic_array_get_ix(dynamic_array_t *array, uint_t index);

void *dynamic_array_init(mem_pool_t *pool, dynamic_array_t *array, uint_t n,
        size_t size);
void dynamic_array_clear(dynamic_array_t *array);


#endif /* __ARRAY_H__ */
