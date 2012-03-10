/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

system_module_t *sys_modules[] = {
    &sys_main_module,
    &sys_net_module,
    NULL
};


int sys_modules_prepare(xpipe_resource_t *resource)
{
    uint_t  i;
    mem_pool_t      *pool;
    system_module_t *mod;

    resource->conf_pool->logger = resource->default_logger;

    for (i = 0; ; i++) {
        mod = *(sys_modules + i);
        if (mod == NULL) {
            break;
        }

        if (mod->logger == NULL) {
            mod->logger = resource->default_logger;
        }

        pool = mem_pool_create(mod->mod_name.data, MODULE_POOL_SIZE,
                               mod->logger);
        if (pool == NULL) {
            log_error(resource->default_logger, 0,
                       "create module \"%s\"'s memory pool failed.", 
                       mod->mod_name.data);
            return MOD_ERROR;
        }

        mod->pool = pool;
    }

    return MOD_OK;
}

int sys_modules_finish(xpipe_resource_t *resource)
{
    return MOD_ERROR;
}
