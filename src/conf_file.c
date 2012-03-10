/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

static int conf_file_init(xpipe_resource_t *resource, conf_file_t *conf,
        logger_t *logger);

static system_module_t *conf_file_get_sys_mod(u_char *name) 
{
    uint_t           i;
    system_module_t *mod;

    for (i = 0; ; i++) {
        mod = *(sys_modules + i);

        if (mod  == NULL) {
            return NULL;
        }

        if (x_strcmp((const char *) name, (const char *) mod->mod_name.data) 
                == 0)
        {
            return mod;
        }
    }
}

static conf_command_t *conf_file_get_command(system_module_t *mod,
        string_t *name)
{
    uint_t          i;
    conf_command_t *cmds, *c;

    cmds = mod->commands;

    for (i = 0; ; i++) {
        c = cmds + i;

        if (c->name.len == 0 || c->mod) {
            return NULL;
        }

        if (name->len == c->name.len 
                && x_strcmp(name->data, c->name.data) == 0) 
        {
            return c;
        }
    }
}

static conf_command_t *conf_file_get_global_cmd(conf_file_t *conf,
        string_t *name)
{
    uint_t           i;
    conf_command_t **cmd;

    for (i = 0; ; i++) {
        cmd = dynamic_array_get_ix(&conf->mod_cmds, i);
        if (cmd == NULL) {
            return NULL;
        }

        if ((*cmd)->name.len == name->len 
            && x_strcmp(name->data, (*cmd)->name.data) == 0)
        {
            return *cmd;
        }
    }
}

static int conf_file_init(xpipe_resource_t *res, conf_file_t *conf,
        logger_t *logger)
{
    int              sys_mod_num, i;
    file_t          *conf_file;
    mem_pool_t      *conf_pool;
    conf_command_t  *cmd, **gcmd;

    sys_mod_num = res->sys_mod_num;

    conf_pool = mem_pool_create((u_char *) "conf_file_pool", 1024, logger);
    if (conf_pool == NULL) {
        log_stderr(0, "create memory conf pool failed,(%s,%d)", 
                   __FILE__, __LINE__);
        return CONF_ERROR;
    }

    conf_file = file_open(conf_pool, xpipe_conf_file_path);
    if (conf_file == NULL) {
        log_stderr(errno, "open configure file \"%s\" error.", 
                   xpipe_conf_file_path);
        return CONF_ERROR;
    }

    conf->file = conf_file;
    conf->blocks = 0;
    conf->pool = conf_pool;

    if (dynamic_array_init(conf_pool, &conf->mod_cmds, 10,
                           sizeof(conf_command_t *)) == NULL)
    {
        log_stderr(0, "init the array of global cmd failed.");
        return CONF_ERROR;
    }

    if (dynamic_array_init(conf_pool, &conf->mod_cfs, sys_mod_num, 
                           sizeof(void *)) == NULL) 
    {
        log_stderr(0, "init the array of local config failed.");
        return CONF_ERROR;
    }

    /* build index of module's commands */
    for (i = 0; ; i++) {
        cmd = sys_main_module.commands + i;
        if (cmd->name.len == 0) {
            break;
        }
        
        if (cmd->mod) {
            gcmd = dynamic_array_push(&conf->mod_cmds);
            if (gcmd == NULL) {
                log_stderr(0, "\"dynamic_array_push\" failed.");
                return CONF_ERROR;
            }

            *gcmd = cmd;
        }
    }

    res->conf_pool = conf_pool;

    return CONF_OK;
}

int conf_file_parser(xpipe_resource_t *resource, conf_file_t *conf,
        logger_t *logger)
{
    void            **elt;
    uint_t            i, k, line;
    u_char            buff[1024], key[256], ch;
    u_char           *p;
    ssize_t           n;
    string_t         *arg, *cmd_name;
    conf_command_t   *cmd;
    dynamic_array_t  *cmd_args;
    system_module_t  *mod;

    enum {
        sw_out = 0,
        sw_comment,
        sw_comment_end,
        sw_mod_name,
        sw_mod_name_end,
        sw_mod_name_end_blank,
        sw_mod_block,
        sw_mod_block_end,
        sw_command,
        sw_command_blank,
        sw_command_end
    } state, old_state;

    if (conf_file_init(resource, conf, logger) == CONF_ERROR) {
        log_error(logger, 0, "Init config file failed.");
        return CONF_ERROR;
    }

    state = sw_out;
    k = 0;
    line = 1;

READ_CONF_FILE:
    n = file_read(conf->file, buff, 1024, FL_DEFAULT_OFFSET);

    if (n == -1) {
        log_stderr(errno, "read file \"%s\" error.", conf->file->name.data);
        return CONF_ERROR;
    } 
    
    if (n == 0) {
        if (state == sw_out || state == sw_mod_block_end) {
            return CONF_OK;
        } 

        log_stderr(0,  "config file \"%s\" is not complete.",
                   conf->file->name.data);
        return CONF_ERROR;
    }

    for (i = 0; i < n; i++) {
        ch = *(buff + i);

        switch (state) {
        case sw_out:
            if (ch == '#') {
                old_state = sw_out;
                state = sw_comment;
            } else if (is_letter(ch)) {
                state = sw_mod_name;
                key[k++] = ch;
            } else if (!is_blank(ch)) {
                log_stderr(0, "the first char of mod name must be letter."
                           "(%d line).", line);
                return CONF_ERROR;
            }

            break;
        case sw_comment:
            if (ch == LF) {
                state = sw_comment_end;
            } else {
                break;
            }
        case sw_comment_end:
            state = old_state;
            break;
        case sw_mod_name:
            if (is_letter(ch) || is_digit(ch) || ch == '_' || ch == '-') {
                key[k++] = ch;
                break;
            } else if (is_blank(ch)) {
                state = sw_mod_name_end_blank;
                key[k] = '\0';
            } else if (ch == '{') {
                state = sw_mod_block;
                key[k] = '\0';
            } else if (ch == '#') {
                old_state = sw_mod_name_end_blank;
                state = sw_comment;
                key[k] = '\0'; 
            } else {
                log_stderr(0, "mod name is invalied.(%d line).", line);
                return CONF_ERROR;
            }
        case sw_mod_name_end:
            mod = conf_file_get_sys_mod((u_char *) key);                        
            if (mod == NULL) {
                log_stderr(0, "\"%s\" module not found.(%d line).", key, line);
                return CONF_ERROR;
            }

            if ((mod->mod_conf = mod->create_conf(conf->pool)) == NULL) {
                log_stderr(0, "\"%s\" module create conf failed.(%d line).", 
                           key, line);
                return CONF_ERROR;
            }

            elt = dynamic_array_push_ix(&conf->mod_cfs, mod->index);
            *elt = mod->mod_conf;

            k = 0;
            conf->blocks++;
            break;
        case sw_mod_name_end_blank:
            if (ch == '{') {
                state = sw_mod_block;
                break;
            } else if (!is_blank(ch)) {
                log_stderr(0, "\"%s\" module's name is error.(%d line).", 
                           key, line);
                return CONF_ERROR;
            }
        case sw_mod_block:
            if (ch == '#') {
                old_state = sw_mod_block;
                state = sw_comment;
                break;
            } else if (is_blank(ch)) {
                /* void */
                break;
            } else if (is_letter(ch)) {
                state = sw_command;
                key[k++] = ch;
                cmd_args = dynamic_array_create(conf->pool, 10,
                                                sizeof(string_t));
                if (cmd_args == NULL) {
                    log_stderr(0, "create \"command\" array failed.");
                    return CONF_ERROR;
                }
                break;
            } else if (ch == '}') {
                state = sw_mod_block_end;
            } else {
                log_stderr(0, "the start char of command must be letter."
                           "(%d line).", line);
                return CONF_ERROR;
            }
        case sw_mod_block_end:
            state = sw_out;
            break;
        case sw_command:
            if (is_blank(ch)) {
                state = sw_command_blank;
                key[k] = '\0';
                
                arg = dynamic_array_push(cmd_args);
                if (arg == NULL) {
                    log_stderr(0, "alloc \"arg\" failed, from \"cmd_args\"");
                    return CONF_ERROR;
                }

                if ((p = pmalloc(conf->pool, k+1)) == NULL) {
                    log_stderr(0, "pmalloc failed, (%s,%d)", __FILE__, __LINE__);
                    return CONF_ERROR;
                }
                
                memcpy(p, key, k+1);

                arg->data = p;
                arg->len = k;

                k = 0;
                break;
            } else if (ch == ';') {
                state = sw_command_end;
                key[k] = '\0';
                
                arg = dynamic_array_push(cmd_args);
                if (arg == NULL) {
                    log_stderr(0, "alloc \"arg\" failed, from \"cmd_args\"");
                    return CONF_ERROR;
                }

                if ((p = pmalloc(conf->pool, k+1)) == NULL) {
                    log_stderr(0, "pmalloc failed, (%s,%d)", 
                               __FILE__, __LINE__);
                    return CONF_ERROR;
                }
                
                memcpy(p, key, k+1);

                arg->data = p;
                arg->len = k;

                k = 0;
            } else {
                key[k++] = ch;
                break;
            }
        case sw_command_end:
CMD_END:
            state = sw_mod_block;
            
            cmd_name = dynamic_array_get_ix(cmd_args, 0);
            cmd = conf_file_get_command(mod, cmd_name);
            if (cmd != NULL) {
                if (cmd->set(cmd_args, mod->mod_conf) == CONF_ERROR) {
                    log_stderr(0, "command \"%s\" set failed.", cmd_name->data);
                    return CONF_ERROR;
                }

                break;
            }

            cmd = conf_file_get_global_cmd(conf, cmd_name);
            if (cmd == NULL) {
                log_stderr(0, "command \"%s\" not found.(%d line).", 
                           cmd_name->data, line);
                return CONF_ERROR;
            }

            if (cmd->set(cmd_args, mod) == CONF_ERROR) {
                log_stderr(0, "command \"%s\" set failed.", cmd_name->data); 
                return CONF_ERROR;
            }

            break;
        case sw_command_blank:
            if (ch == ';') {
                state = sw_command_end;
                goto CMD_END;
            } else if (is_blank(ch)) {
                /* void */
            } else {
                state = sw_command;
                key[k++] = ch;
            }

            break;
        } // end of state

        if (ch == LF) {
            line++;
        }
    }

    goto READ_CONF_FILE;
}
