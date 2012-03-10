/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include "config.h"
#include "system.h"

#define MAX_SIZE_OF_CHUNK 4096

#define REQUEST_POOL_SIZE       (1024 * 1024)
#define CONNECTION_POOL_SIZE    2048
#define CONF_POOL_SIZE          1024
#define MODULE_POOL_SIZE        2048

typedef struct mem_pool_large_s mem_pool_large_t;

struct mem_pool_large_s {
    u_char           *data;
    mem_pool_large_t *next;
};

typedef struct mem_pool_chunk_s mem_pool_chunk_t;

struct mem_pool_chunk_s {
    int               fail;
    u_char           *last;
    u_char           *end;
    mem_pool_chunk_t *next;
};

struct mem_pool_s {
    string_t          name;
    size_t            total;
    size_t            max;
    mem_pool_chunk_t *current;
    mem_pool_large_t *large;
    logger_t         *logger;

    mem_pool_chunk_t  chunk; 
};


mem_pool_t *mem_pool_create(u_char *name, size_t size, logger_t *logger);
void mem_pool_destroy(mem_pool_t *pool);
void mem_pool_clear(mem_pool_t *pool);

void *pmalloc(mem_pool_t *pool, size_t size);
void *pcalloc(mem_pool_t *pool, size_t size);
int pfree_large(mem_pool_t *pool, void *p);

#endif /* __MEM_POOL_H__ */
