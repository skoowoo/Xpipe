/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"


static void *alloc_large_mem(mem_pool_t *pool, size_t size)
{
    int               n;
    u_char           *p;
    mem_pool_large_t *large;

    p = (u_char *) malloc(size);
    if (p == NULL) {
        log_error(pool->logger, 0, "\"alloc_large_mem - data\" failed.");
        return NULL;
    }

    for (n = 0, large = pool->large; large ; large = large->next) {
        if (large->data == NULL) {
            large->data = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = (mem_pool_large_t *) pmalloc(pool, sizeof(mem_pool_large_t));
    if (large == NULL) {
        log_error(pool->logger, 0, "\"alloc_large_mem - header\" failed.");
        free(p);
        return NULL;
    }

    large->data = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

static void *add_mem_chunk(mem_pool_t *pool, size_t size)
{
    size_t            alloc_size;
    mem_pool_chunk_t *p, *new_chunk;

    alloc_size = (size > pool->total ? size : pool->total);

    new_chunk = (mem_pool_chunk_t *) malloc(alloc_size);
    if (new_chunk == NULL) {
        log_error(pool->logger, 0, "\"add_mem_chunk\" failed.");
        return NULL;
    }

    new_chunk->end = (u_char *) new_chunk + alloc_size;
    new_chunk->last = (u_char *) new_chunk + sizeof(mem_pool_chunk_t) + size;
    new_chunk->fail = 0;
    new_chunk->next = NULL;

    pool->total += alloc_size;

    for (p = pool->current; p->next ; p = p->next) {
        if (p->fail++ > 4) {
            pool->current = p->next;
        }
    }

    /* last node */
    p->fail++; 
    p->next = new_chunk;

    return (u_char *) new_chunk + sizeof(mem_pool_chunk_t);
}

mem_pool_t *mem_pool_create(u_char *name, size_t size, logger_t *logger)
{
    mem_pool_t *pool;

    pool = (mem_pool_t *) malloc(size);
    if (pool == NULL) {
        log_error(logger, 0, "\"create_mem_pool\" failed.");
        return NULL;
    }

    pool->name.data = name;
    pool->name.len = x_strlen(name);

    pool->chunk.end = (u_char *) pool + size;
    pool->chunk.last = (u_char *) pool + sizeof(mem_pool_t);
    pool->chunk.fail = 0;
    pool->chunk.next = NULL;

    pool->total = size;
    pool->max = size < MAX_SIZE_OF_CHUNK ? size : MAX_SIZE_OF_CHUNK;
    pool->current = &pool->chunk;
    pool->large = NULL;
    pool->logger = logger;

    return pool;
}

void mem_pool_destroy(mem_pool_t *pool)
{
    mem_pool_chunk_t *p;
    mem_pool_large_t *l;

    /* release large memory at first */
    for (l = pool->large; l; l = pool->large) {
        pool->large = l->next;
        free((void *) l);
    }

    for (p = pool->chunk.next; p; p = pool->chunk.next) {
        pool->chunk.next = p->next;
        free((void *) p);
    }

    free((void *) pool);
}

void mem_pool_clear(mem_pool_t *pool)
{
    mem_pool_chunk_t *p;
    mem_pool_large_t *l;

    /* release large memory at first */
    for (l = pool->large; l; l = pool->large) {
        pool->large = l->next;
        free((void *) l);
    }

    pool->current = &pool->chunk;

    for (p = pool->current; p; p = p->next) {
        p->fail = 0;
        p->last = (u_char *) p + sizeof(mem_pool_chunk_t);
    }
}

void *pmalloc(mem_pool_t *pool, size_t size)
{
    u_char           *p;
    mem_pool_chunk_t *chunk;

    if (size <= pool->max) {
        chunk = pool->current;

        do {
            p = chunk->last;
            if ((u_char *) chunk->end - p >= size) {
                chunk->last += size;
                return p;
            }

            chunk = chunk->next;
        } while (chunk);

        return add_mem_chunk(pool, size);
    } 

    return alloc_large_mem(pool, size);
}

void *pcalloc(mem_pool_t *pool, size_t size)
{
    void *p;

    p = pmalloc(pool, size);
    if (p) {
        memset(p, 0, size);
    }

    return p;
}

int pfree_large(mem_pool_t *pool, void *p)
{
    mem_pool_large_t *l;

    for (l = pool->large; l ; l = l->next) {
        if (l->data == p) {
            free((void *) p);
            l->data = NULL;
            return XPE_OK;
        }
    }

    return XPE_ERROR;
}
