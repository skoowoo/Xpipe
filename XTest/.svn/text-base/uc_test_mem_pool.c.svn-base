#include "core/xtest.h"

#include "../src/xpipe.h"
#include "../src/config.h"
#include "../src/system.h"


static int prepare(void);
static int run(void);
static int finish(void);


unit_cases_t test_mem_pool = {
    "test_mem_pool",
    prepare,
    run,
    finish
};

static file_t       *file;

static logger_t     *logger;
static mem_pool_t   *pool;

static int prepare(void)
{
    file = malloc(sizeof(file_t));
    if (file == NULL) {
        fprintf(stderr, "malloc \"file_t\"\n");
        return TEST_ERROR; 
    }   

    file->fd = FL_INVALID_FD; 
    set_string(&file->name, "/tmp/test.log");
    file->offset = FL_BEGIN_OFFSET;

    file->fd = open_file_fd(file->name.data, FL_APPEND|FL_CREATE, 0644);
    if (file->fd == FL_INVALID_FD) {
        perror("open file");
        return TEST_ERROR; 
    }   

    logger = malloc(sizeof(logger_t));
    if (logger == NULL) {
        fprintf(stderr, "malloc \"logger_t\"\n");
        return TEST_ERROR; 
    }   

    logger->file = file;
    logger->level = LOG_LEVEL_DEBUG;

    return TEST_OK;
}

static int run(void) 
{
    TEST_CASE("create_mem_pool(1024)") 
    {
        pool = mem_pool_create((u_char *) "mem_pool_test", 1024, logger);      

        ASSERT_NOT_NULL(pool);

        ASSERT_EQ(pool->total, 1024);
        ASSERT_EQ(pool->max, 1024);
        ASSERT_EQ(pool->current, &pool->chunk);
        ASSERT_EQ(pool->large, NULL);
        ASSERT_EQ(pool->logger, logger);

        ASSERT_EQ(pool->chunk.fail, 0);
        ASSERT_EQ(pool->chunk.end, (u_char *) pool + 1024);
        ASSERT_EQ(pool->chunk.last, (u_char *) pool + sizeof(mem_pool_t));
        ASSERT_EQ(pool->chunk.next, NULL);
    }

    /* it depend on the above case */
    TEST_CASE("pmalloc")
    {
        u_char *p;

        p = pmalloc(pool, 10);

        ASSERT_NOT_NULL(pool);
        ASSERT_EQ(pool->current, &pool->chunk);
        ASSERT_EQ(pool->chunk.last, (u_char *) pool + sizeof(mem_pool_t) + 10);

        p = pmalloc(pool, 512);

        ASSERT_NOT_NULL(pool);
        ASSERT_EQ(pool->current, &pool->chunk);
        ASSERT_EQ(pool->chunk.last, (u_char *) pool + sizeof(mem_pool_t) + 522);
    }

    /* it depend on the above cases */
    TEST_CASE("pmalloc (add chunk)")
    {
        u_char *p;

        p = pmalloc(pool, 1024);

        ASSERT_NOT_NULL(p);
        ASSERT_EQ(pool->current, &pool->chunk);
        ASSERT_EQ(pool->chunk.last, (u_char *) pool + sizeof(mem_pool_t) + 522);
        ASSERT_NOT_NULL(pool->chunk.next);
        ASSERT_EQ(pool->chunk.next->last, 
                  (u_char *) (pool->chunk.next) + sizeof(mem_pool_chunk_t) + 1024);

        ASSERT_EQ(pool->chunk.fail, 1);
        ASSERT_EQ(pool->chunk.next->fail, 0);
        ASSERT_EQ(pool->total, 2048);
    }

    TEST_CASE("pmalloc large")
    {

    }

    TEST_CASE("pcalloc")
    {

    }

    return TEST_OK;
}

static int finish(void)
{
    free(file);
    free(logger);

    mem_pool_clear(pool);
    mem_pool_destroy(pool);

    return TEST_OK;
}
