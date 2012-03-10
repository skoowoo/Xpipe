/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __MOD_MANAGER_H__
#define __MOD_MANAGER_H__

#include "config.h"
#include "system.h"

#define MOD_OK      0
#define MOD_ERROR   -1

typedef void* (*create_mod_conf_fp) (mem_pool_t *pool);
typedef int (*init_mod_fp) (system_module_t *mod);
typedef int (*process_mod_fp) (system_module_t *mod);
typedef int (*finish_mod_fp) (system_module_t *mod);


struct system_module_s {
    string_t            mod_name;
    conf_command_t     *commands;    
    create_mod_conf_fp  create_conf;
    init_mod_fp         init_mod;
    process_mod_fp      process_mod;
    finish_mod_fp       finish_mod;
    int                 index;
    void               *mod_conf;
    mem_pool_t         *pool;
    logger_t           *logger;
};

#define sys_mod_padding -1,NULL,NULL,NULL
#define sys_null_module { {0, NULL}, NULL, NULL, -1, NULL,NULL }


extern system_module_t sys_main_module;
extern system_module_t sys_net_module;

extern system_module_t *sys_modules[];

#define is_sys_main_mod(name) (strcmp((const char *) (name)->data, "main") == 0)
#define is_sys_net_mod(name) (strcmp((const char *) (name)->data, "net") == 0)

int sys_modules_prepare(xpipe_resource_t *resource);
int sys_modules_finish(xpipe_resource_t *resource);

#endif /* __MOD_MANAGER_H__ */
