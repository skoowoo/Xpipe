/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "config.h"
#include "system.h"

#define TCP_SRV_OK       0
#define TCP_SRV_ERROR   -1


#define BUFFER_MIN_SIZE 1024
#define BUFFER_MAX_SIZE 1024 * 1204 * 10

typedef struct sockaddr_in xpe_sockaddr_in;
typedef struct sockaddr xpe_sockaddr;

typedef struct {
    u_char *buffer;
    u_char *end;
    u_char *last;
    void   *next;
} buffer_t;

struct tcp_request_s {
    buffer_t            *buffers;
    buffer_t            *last_buffer;

    buffer_t            *response;
    buffer_t            *last_rep_buf;

    tcp_connection_t    *conn;

    protocol_t          *protocol;
    protocol_parse_fp    parser;

    int                  done;
    int                  finish;
    int                  error;

    mem_pool_t          *pool;
    logger_t            *logger;    
};

struct tcp_connection_s {
    void                *next;
    int                  conn_fd;
    int                  events;
    int                  active_events;
    int                  dead_events;
    int                  add_events;
    int                  close;
    int                  is_listen;
    
    event_handler_fp     read_event_handler;
    event_handler_fp     write_event_handler;

    tcp_server_t        *server;
    tcp_request_t       *request;
    
    string_t             client_addr;
    int                  client_port;

    void                *queue;

    mem_pool_t          *pool;
    logger_t            *logger;
};

struct tcp_server_s {
    uint_t               port;
    uint_t               connections;

    int                  sock_fd;
    tcp_connection_t    *connection_pool;
    xpe_sockaddr_in      sa;
    net_event_driver_t  *event_driver;

    int                  nodelay;
    uint_t               request_buf_size;

    mem_pool_t          *pool;
    logger_t            *logger;
};


int tcp_server_init(tcp_server_t *server);

int tcp_server_create_socket(tcp_server_t *server);
int tcp_server_listen(tcp_server_t *server, int s_fd);
int tcp_server_accept(tcp_connection_t *conn);
int tcp_server_recv(tcp_connection_t *conn);
int tcp_server_send(tcp_connection_t *conn);

int connection_pool_init(tcp_server_t *server);
tcp_connection_t *tcp_get_connection(tcp_server_t *server, int fd);
void tcp_free_connection(tcp_server_t *server, tcp_connection_t *c);

tcp_request_t *tcp_request_init(tcp_connection_t *conn);
int tcp_request_process(tcp_request_t *r, int ret);
int tcp_request_finish(tcp_request_t *r);
void tcp_respone_generate(tcp_request_t *r);

int set_nonblock(int fd);
int set_nodelay(int fd);
#endif  /* __TCP_SERVER_H__ */
