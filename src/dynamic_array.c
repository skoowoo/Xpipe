/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"


dynamic_array_t *dynamic_array_create(mem_pool_t *pool, uint_t n, uint_t size)
{
    dynamic_array_t *a;

    a = pmalloc(pool, sizeof(dynamic_array_t));
    if (a == NULL) {
        log_error(pool->logger, 0, "create array failed,(%s,%d)",
                  __FILE__, __LINE__);
        return NULL;
    } 

    a->elts = pmalloc(pool, n * size);
    if (a->elts == NULL) {
        log_error(pool->logger, 0, "create array's elts failed,(%s,%d)",
                  __FILE__, __LINE__);
        return NULL;
    }

    a->nalloc = n;
    a->nelts = 0;
    a->size = size;
    a->pool = pool;

    return a;
}

void dynamic_array_destroy(dynamic_array_t *array)
{

}

void *dynamic_array_push(dynamic_array_t *array)
{
    void        *new;
    size_t       size;
    mem_pool_t  *pool;

    if (array->nelts == array->nalloc) {
        size = array->size * array->nalloc;
        pool = array->pool;

        new = pmalloc(pool, 2 * size);
        if (new == NULL) {
            log_error(pool->logger, 0, "extend array failed,(%s,%d).",
                      __FILE__, __LINE__);
            return NULL;
        }

        memcpy(new, array->elts, size);

        array->nalloc *= 2;
        array->elts = new;
    }

    return (u_char *) array->elts + (array->nelts++) * array->size;
}

void *dynamic_array_push_ix(dynamic_array_t *array, uint_t index)
{
    mem_pool_t  *pool;

    pool = array->pool;

    if (index >= array->nalloc) {
        log_error(pool->logger, 0, "index out of array length,(%d,%d)",
                  index, array->nalloc);
        return NULL;
    }

    return (u_char *) array->elts + index * array->size;
}

void *dynamic_array_get_ix(dynamic_array_t *array, uint_t index)
{
    mem_pool_t  *pool;
    pool = array->pool;

    if (index >= array->nelts) {
        log_error(pool->logger, 0, "index out of array elements,(%d,%d)",
                  index, array->nelts);
        return NULL;
    }

    return (u_char *) array->elts + index * array->size;
}
void *dynamic_array_init(mem_pool_t *pool, dynamic_array_t *array, uint_t n,
        size_t size)
{
    array->elts = pmalloc(pool, n * size);
    if (array->elts == NULL) {
        return NULL;
    }

    array->nalloc = n;
    array->nelts = 0;
    array->size = size;
    array->pool = pool;

    return array;
}

void dynamic_array_clear(dynamic_array_t *array)
{

}
