/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

#ifdef USE_BACKTRACE
static void sig_backtrace_handler(int sig, siginfo_t *info, void *context);
#else
static void sig_term_handler(int signo);
#endif
static int setup_signals();
static int create_default_logger(xpipe_resource_t *resource);
static int system_modules_init(xpipe_resource_t *resource);
static void system_modules_working(xpipe_resource_t *resource);
static int process_daemon(int daemon, file_t *pid_file, logger_t *logger);

static void *create_main_loc_conf(mem_pool_t *pool);
static int init_main_mod(system_module_t *mod);

static int cmd_daemon_set(dynamic_array_t *args, void *mod_conf);
static int cmd_pid_set(dynamic_array_t *args, void *mod_conf);
static int cmd_log_set(dynamic_array_t *args, void *mod_conf);


typedef struct {
    u_char      daemon; 
    file_t      pid_file;
    string_t    log;
} xpipe_main_mod_conf_t;

static conf_command_t commands[] = {
    { 0, xstring("daemon"), cmd_daemon_set },
    { 0, xstring("pid"), cmd_pid_set },
    { 1, xstring("log"), cmd_log_set },
    conf_command_null
};

system_module_t sys_main_module = {
    xstring("main"),
    commands,
    create_main_loc_conf,
    init_main_mod,
    NULL,
    NULL,
    sys_mod_padding
};

xpipe_resource_t xpipe_resource;


static void *create_main_loc_conf(mem_pool_t *pool)
{
    xpipe_main_mod_conf_t *cf;

    cf = pcalloc(pool, sizeof(xpipe_main_mod_conf_t));
    if (cf == NULL) {
        log_error(pool->logger, errno, "Create main module config error.");
        return NULL;
    }

    cf->pid_file.fd = FL_INVALID_FD;
    cf->pid_file.name.data = (u_char *) default_pid_file_full_path;
    cf->pid_file.name.len = x_strlen(default_pid_file_full_path);

    return cf;
}

static int init_main_mod(system_module_t *mod)
{
    if (setup_signals() == XPE_ERROR) {
        return XPE_ERROR;
    }

    return MOD_OK;
}

static int cmd_daemon_set(dynamic_array_t *args, void *mod_conf)
{
    string_t              *arg;
    xpipe_main_mod_conf_t *cf;

    cf = (xpipe_main_mod_conf_t *) mod_conf;

    if (args->nelts != 2) {
        log_error(args->pool->logger, 0, "the args of \"daemon\" is error.");
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 1);
    if (arg == NULL) {
        return CONF_ERROR;
    }

    cf->daemon = x_strcmp((const char *) arg->data, "off") == 0 ? 0 : 1;

    return CONF_OK;
}

static int cmd_pid_set(dynamic_array_t *args, void *mod_conf)
{
    string_t        *arg;
    mem_pool_t      *pool;

    xpipe_main_mod_conf_t *cf;

    cf = (xpipe_main_mod_conf_t *) mod_conf;
    pool = args->pool; 

    if (args->nelts != 2) {
        log_error(args->pool->logger, 0, "the args of \"pid\" is error.");
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 1);
    if (arg == NULL) {
        return CONF_ERROR;
    }

    cf->pid_file.fd = FL_INVALID_FD;

    if (is_full_file_name(arg)) {
        cf->pid_file.name.data = arg->data;
        cf->pid_file.name.len = arg->len;
    } else {
        cf->pid_file.name.data = pcalloc(pool, strlen(xpipe_install_dir_path) 
                                        + arg->len + 2);
        if (cf->pid_file.name.data == NULL) {
            return CONF_ERROR; 
        }

        make_full_file_name(cf->pid_file.name.data, arg->data, arg->len);
        cf->pid_file.name.len = x_strlen(cf->pid_file.name.data);
    }

    return CONF_OK;
}

static int cmd_log_set(dynamic_array_t *args, void *mod_conf)
{
    int              level;
    file_t          *file;
    string_t        *arg;
    mem_pool_t      *pool;
    system_module_t *mod;

    mod = (system_module_t *) mod_conf; 
    pool = args->pool; 

    if (args->nelts != 3) {
        log_error(args->pool->logger, 0, "the args of \"log\" is error.");
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 1);
    if (arg == NULL) {
        return CONF_ERROR;
    }

    file = pmalloc(pool, sizeof(file_t));
    if (file == NULL) {
        return CONF_ERROR;
    }

    file->fd = FL_INVALID_FD;

    if (is_full_file_name(arg)) {
        file->name.data = arg->data;
        file->name.len = arg->len;
    } else {
        file->name.data = pcalloc(pool, strlen(xpipe_install_dir_path) 
                                        + arg->len + 2);
        if (file->name.data == NULL) {
            return CONF_ERROR; 
        }

        make_full_file_name(file->name.data, arg->data, arg->len);
        file->name.len = x_strlen(file->name.data);
    }

    file->fd = open_file_fd(file->name.data, FL_APPEND|FL_CREATE, 0644);
    if (file->fd == FL_INVALID_FD) {
        log_error(args->pool->logger, errno, "open \"%s\" error", arg->data);
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 2);
    if (arg == NULL) {
        return CONF_ERROR;
    }

    if (x_strcmp(arg->data, "error") == 0) {
       level = LOG_LEVEL_ERROR;
    } else if (x_strcmp(arg->data, "warn") == 0) {
        level = LOG_LEVEL_WARN;
    } else if (x_strcmp(arg->data, "info") == 0) {
        level = LOG_LEVEL_INFO;
    } else if (x_strcmp(arg->data, "debug") == 0) {
        level = LOG_LEVEL_DEBUG;
    } else if (x_strcmp(arg->data, "stdout") == 0) {
        level = LOG_LEVEL_STDOUT;
    } else {
        log_error(args->pool->logger, 0, "\"%s\" is invalid arg.", arg->data);
        return CONF_ERROR;
    }

    mod->logger = log_create(args->pool, level, file); 
    if (mod->logger == NULL) {
        log_error(args->pool->logger, 0, "create logger failed by \"log\" cmd");
        return CONF_ERROR;
    }

    return CONF_OK;
}

#ifndef UNIT_TEST
int main(int argc, char **args) 
{
    uint_t                   i, sys_mod_num;
    logger_t                 log_stdout;
    conf_file_t              conf;
    system_module_t         *mod;
    xpipe_main_mod_conf_t   *cf;

    timer_init();

    sys_mod_num = 0;
    for (i = 0; ; i++) {
        mod = *(sys_modules + i);    
        if (mod == NULL) {
            break;
        }

        mod->index = i;
        sys_mod_num++;
    }

    xpipe_resource.sys_mod_num = sys_mod_num;

    log_stdout.level = LOG_LEVEL_INFO;
    log_stdout.file = NULL; 

    if (conf_file_parser(&xpipe_resource, &conf, &log_stdout) == CONF_ERROR) {
        return XPE_ERROR;
    } 

    /* build some resource */
    xpipe_resource.stop = 0;
    xpipe_resource.conf = &conf;
    xpipe_resource.log_stdout = &log_stdout;
    xpipe_resource.main_logger = sys_main_module.logger;
    if (create_default_logger(&xpipe_resource) == XPE_ERROR) {
        return XPE_ERROR;
    }
    
    log_info(xpipe_resource.default_logger, 0, 
             "finish building resource, begin to prepare system modules.");
    /* module prepare */
    if (sys_modules_prepare(&xpipe_resource) == MOD_ERROR) {
        log_error(xpipe_resource.default_logger, 0,
                  "prepare all system modules failed.");
        return XPE_ERROR;
    }

    /* init  modules */
    if (system_modules_init(&xpipe_resource) == XPE_ERROR) {
        return XPE_ERROR;
    }

    cf = (xpipe_main_mod_conf_t *) sys_main_module.mod_conf;
    if (process_daemon(cf->daemon, &cf->pid_file, xpipe_resource.default_logger)
            == XPE_ERROR)
    {
        return XPE_ERROR;
    }

    /* process modules */
    system_modules_working(&xpipe_resource);
    
    return 0;
}
#endif

#ifdef USE_BACKTRACE

static void sig_backtrace_handler(int sig, siginfo_t *info, void *context)
{
    int                i, size;
    void              *buffer[100];
    u_char           **msgs;
    struct sigaction   act;

    size = backtrace(buffer, 100);

    msgs = (u_char **) backtrace_symbols(buffer, size);
    if (msgs == NULL) {
        goto process_exit;
    }

    for (i = 0; i < size; i++) {
        log_error(xpipe_resource.default_logger, 0, "%s", (char *) msgs[i]);
    }

    log_printf("aaaaaaaa");
process_exit:
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
    act.sa_handler = SIG_DFL;
    sigaction(sig, &act, NULL);
    kill(getpid(), sig);
}

#else

static void sig_term_handler(int signo)
{
    log_warn(xpipe_resource.default_logger, 0, 
             "Receive \"SIGTERM\" signal, process will exit.");
    xpipe_resource.stop = 1;
}

#endif

static int setup_signals() 
{
    struct sigaction act;

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;

#ifdef USE_BACKTRACE

    act.sa_flags |= SA_SIGINFO;

#ifdef SA_RESTART
    act.sa_flags |= SA_RESTART; 
#endif

    act.sa_sigaction = sig_backtrace_handler;

    if (sigaction(SIGSEGV, &act, NULL) == -1) {
        return XPE_ERROR;
    }

    if (sigaction(SIGBUS, &act, NULL) == -1) {
        return XPE_ERROR;
    }

    if (sigaction(SIGFPE, &act, NULL) == -1) {
        return XPE_ERROR;
    }

    if (sigaction(SIGILL, &act, NULL) == -1) {
        return XPE_ERROR;
    }
#else

#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif

    act.sa_handler = sig_term_handler;

    if (sigaction(SIGTERM, &act, NULL) == -1) {
        return XPE_ERROR;
    }
#endif

    return XPE_OK;
}

static int create_default_logger(xpipe_resource_t *resource)
{
    file_t          *file;
    logger_t        *logger;
    mem_pool_t      *pool;

    if (resource->main_logger != NULL) {
        resource->default_logger = resource->main_logger;
        return XPE_OK;
    }

    pool = resource->conf_pool; 

    file = pmalloc(pool, sizeof(file_t));
    if (file == NULL) {
        return XPE_ERROR;
    }

    file->fd = FL_INVALID_FD;
    file->name.data = (u_char *) default_logger_full_path;
    file->name.len = x_strlen(default_logger_full_path);

    file->fd = open_file_fd(file->name.data, FL_APPEND, 0644);
    if (file->fd == FL_INVALID_FD) {
        log_error(resource->log_stdout, errno, "open \"%s\" error", 
                  file->name.data);
        return XPE_ERROR;
    }

    logger = log_create(pool, LOG_LEVEL_INFO, file); 
    if (logger == NULL) {
        log_error(resource->log_stdout, 0, 
                  "create default logger failed.");
        return XPE_ERROR;
    }

    return XPE_OK;
}

static int system_modules_init(xpipe_resource_t *resource)
{
    int              i;
    system_module_t *mod;

    for (i = 0; ; i++) {
        mod = *(sys_modules + i);
        if (mod == NULL) {
            break;
        }

        if (mod->init_mod && mod->init_mod(mod) == MOD_ERROR) {
            return XPE_ERROR;
        }

        log_info(resource->default_logger, 0,
                 "module \"%s\" init successfully.", mod->mod_name.data);
    } 

    return XPE_OK;
}

static void system_modules_working(xpipe_resource_t *resource)
{
    int              i;
    system_module_t *mod;

    while (!resource->stop) {
        for (i = 0; ; i++) {
            mod = *(sys_modules + i);
            if (mod == NULL) {
                break;
            }

            if (mod->process_mod != NULL 
                    && mod->process_mod(mod) == MOD_ERROR) 
            {
                log_error(resource->default_logger, 0,
                          "process module \"%s\" failed.", mod->mod_name.data);
            }
        }

        timer_update();
    }
}

static int process_daemon(int daemon, file_t *pid_file, logger_t *logger)
{
    int    fd, len;
    u_char buffer[32];

    if (daemon == 0) {
        return XPE_OK;
    }
    
    if (fork() != 0) {
        exit(0);
    }

    setsid();

    if ((fd = open_file_fd("/dev/null", FL_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }

    pid_file->fd = open_file_fd(pid_file->name.data, FL_WRONLY|FL_CREATE, 0644);
    if (pid_file->fd == FL_INVALID_FD) {
        log_error(logger, errno, "Open \"%s\" error", pid_file->name.data);
        return XPE_ERROR;
    }

    memset(buffer, 0, 32);
    sprintf((char *) buffer, "%d", getpid());
    len = x_strlen(buffer);

    if (write_fd(pid_file->fd, buffer, len) != len) {
        log_error(logger, errno, "Write pid into file failed.");
        return XPE_ERROR;
    }

    return XPE_OK;
}
