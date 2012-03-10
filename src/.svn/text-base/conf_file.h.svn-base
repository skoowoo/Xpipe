/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __CONF_PARSE_H__
#define __CONF_PARSE_H__

#include "config.h"
#include "system.h"

#define xpipe_conf_file_path "conf/xpipe.conf"

#define CONF_OK      0
#define CONF_ERROR  -1

typedef int (*cmd_set_fp) (dynamic_array_t *args, void *mod_conf);

typedef struct {
    u_char      mod;
    string_t    name;
    cmd_set_fp  set;    
} conf_command_t;

#define conf_command_null  {0, {0, NULL}, NULL}

struct conf_file_s {
    file_t          *file;
    uint_t           blocks;
    dynamic_array_t  mod_cmds;   /* element is 'conf_command_t *' */
    dynamic_array_t  mod_cfs;       /* element is 'void *' */
    mem_pool_t      *pool;
};


int conf_file_parser(xpipe_resource_t *resource, conf_file_t *conf,
        logger_t *logger);

#endif /* __CONF_PARSE_H__ */
