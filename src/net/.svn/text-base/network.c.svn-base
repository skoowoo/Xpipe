/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

#define is_valid_port(text)  is_positive_integer(text)

static void *create_net_mod_conf(mem_pool_t *pool);
static int init_network_mod(system_module_t *mod);
static int process_network_mod(system_module_t *mod);
static int finish_network_mod(system_module_t *mod);

static int cmd_listen_set(dynamic_array_t *args, void *mod_conf);
static int cmd_connecions_set(dynamic_array_t *args, void *mod_conf);
static int cmd_nodelay_set(dynamic_array_t *args, void *mod_conf);
static int cmd_request_buffer_size_set(dynamic_array_t *args, void *mod_conf);


typedef struct {
    tcp_server_t        *server;
    net_event_driver_t  *event_driver;
} network_t;

typedef struct {
    uint_t      port;
    uint_t      conns;
    int         nodelay;
    uint_t      request_buffer_size;
    network_t  *netwk;
} xpipe_net_mod_conf_t;

static conf_command_t commands[] = {
    { 0, xstring("listen"), cmd_listen_set },
    { 0, xstring("connections"), cmd_connecions_set },
    { 0, xstring("nodelay"), cmd_nodelay_set },
    { 0, xstring("request_buffer_size"), cmd_request_buffer_size_set },
    conf_command_null
};

system_module_t sys_net_module = {
    xstring("net"),
    commands,
    create_net_mod_conf,
    init_network_mod,
    process_network_mod,
    finish_network_mod,
    sys_mod_padding
};


static void *create_net_mod_conf(mem_pool_t *pool)
{
    network_t            *netwk;
    xpipe_net_mod_conf_t *cf;

    cf = pcalloc(pool, sizeof(xpipe_net_mod_conf_t));
    if (cf == NULL) {
        log_error(pool->logger, 0, "create \"net\" module config error.");
        return NULL;
    }

    netwk = pmalloc(pool, sizeof(network_t));
    if (netwk == NULL) {
        log_error(pool->logger, 0, "\"pmalloc\" memory failed.");
        return NULL;
    }

    cf->netwk = netwk;

    return cf;
}

static int init_network_mod(system_module_t *mod)
{
    tcp_server_t         *server;
    tcp_connection_t     *conn;
    net_event_driver_t   *event_driver;
    xpipe_net_mod_conf_t *cf;

    cf = (xpipe_net_mod_conf_t *) mod->mod_conf;

    /**
     * init the layer of event driver 
     */
    event_driver = pmalloc(mod->pool, sizeof(net_event_driver_t));
    if (event_driver == NULL) {
        log_error(mod->logger, 0, "\"pmalloc\" memory failed.");
        return MOD_ERROR;
    }

    event_driver->size = cf->conns / 2;
    event_driver->pool = mod->pool;
    event_driver->logger = mod->logger;

    cf->netwk->event_driver = event_driver;

    if (net_event_init(event_driver) == EVENT_ERROR) {
        return MOD_ERROR;
    }

    log_info(mod->logger, 0, "event driver has been init successfully.");


    /**
     * init the layer of tcp server
     */
    server = pmalloc(mod->pool, sizeof(tcp_server_t));
    if (server == NULL) {
        log_error(mod->logger, 0, "\"pmalloc\" memory failed.");
        return MOD_ERROR;
    }

    server->port = cf->port;
    server->connections = cf->conns;
    server->event_driver = event_driver;
    server->nodelay = cf->nodelay;
    server->request_buf_size = cf->request_buffer_size;
    server->pool = mod->pool;
    server->logger = mod->logger;

    cf->netwk->server = server;

    if (tcp_server_init(server) == TCP_SRV_ERROR) {
        return MOD_ERROR;
    }

    log_info(mod->logger, 0, 
             "server init successfully, port: %d, max connections: %d",
             server->port, server->connections);


    /**
     * Start to listen the socket
     */
    conn = tcp_get_connection(server, server->sock_fd);    
    if (conn == NULL) {
        log_error(mod->logger, 0,
                  "get connection from pool failed, pool may be full.");
        return MOD_ERROR;
    }

    conn->is_listen = 1;

    /* add socket fd's read event to event driver */
    if (add_event(event_driver, conn, EV_READ_EVENT, tcp_server_accept) 
            == EVENT_ERROR)
    {
        log_error(mod->logger, 0,
                  "add the read event of listen %d to event driver failed.",
                  conn->conn_fd);
        close(conn->conn_fd);
        tcp_free_connection(server, conn);
        return MOD_ERROR;
    }

    log_info(mod->logger, 0, 
            "socket fd(%d) has been added to event driver, server is listening",
             conn->conn_fd);

    return MOD_OK;
}

static int process_network_mod(system_module_t *mod)
{
    xpipe_net_mod_conf_t *cf;

    cf = (xpipe_net_mod_conf_t *) mod->mod_conf;

    if (process_events(cf->netwk->event_driver) == EVENT_ERROR) {
        return MOD_ERROR;
    }

    return MOD_OK;
}

static int finish_network_mod(system_module_t *mod)
{
    return MOD_OK;
}

static int cmd_listen_set(dynamic_array_t *args, void *mod_conf)
{
    string_t             *arg;
    xpipe_net_mod_conf_t *cf;

    cf = (xpipe_net_mod_conf_t *) mod_conf;

    if (args->nelts != 2) {
        log_error(args->pool->logger, 0, "the args of \"listen\" is error.");
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 1);
    if (arg == NULL) {
        return CONF_ERROR;
    }

    if (!is_valid_port(arg)) {
        log_error(args->pool->logger, 0, 
                  "\"%s\" is a invalid port.", arg->data);
        return CONF_ERROR;
    }

    cf->port = x_atoi(arg->data);
    
    return CONF_OK;
}

static int cmd_connecions_set(dynamic_array_t *args, void *mod_conf)
{
    string_t             *arg;
    xpipe_net_mod_conf_t *cf;

    cf = (xpipe_net_mod_conf_t *) mod_conf;

    if (args->nelts != 2) {
        log_error(args->pool->logger, 0, 
                  "the args of \"connections\" is error.");
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 1);
    if (arg == NULL) {
        return CONF_ERROR;
    }

    if (!is_positive_integer(arg)) {
        log_error(args->pool->logger, 0, "\"%s\" is not a integer.", arg->data);
        return CONF_ERROR;
    }

    cf->conns = x_atoi(arg->data);
    return CONF_OK;
}

static int cmd_nodelay_set(dynamic_array_t *args, void *mod_conf)
{
    xpipe_net_mod_conf_t *cf;

    cf = (xpipe_net_mod_conf_t *) mod_conf;

    if (args->nelts != 1) {
        log_error(args->pool->logger, 0, 
                  "the args of \"nodelay\" is error.");
        return CONF_ERROR;
    }

    cf->nodelay = 1;

    return CONF_OK;
}

static int cmd_request_buffer_size_set(dynamic_array_t *args, void *mod_conf)
{
    string_t             *arg;
    xpipe_net_mod_conf_t *cf;

    cf = (xpipe_net_mod_conf_t *) mod_conf;

    if (args->nelts != 2) {
        log_error(args->pool->logger, 0,
                  "the args of \"request_buffer_size\" is error.");
        return CONF_ERROR;
    }

    arg = dynamic_array_get_ix(args, 1); 
    if (arg == NULL) {
        return CONF_ERROR;
    }

    if (!is_positive_integer(arg)) {
        log_error(args->pool->logger, 0, "\"%s\" is not a integer.", arg->data);
        return CONF_ERROR;
    }

    cf->request_buffer_size = x_atoi(arg->data);

    return CONF_OK;
}
