/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

int tcp_server_init(tcp_server_t *server)
{
    int s_fd;

    if ((s_fd = tcp_server_create_socket(server)) == TCP_SRV_ERROR) {
        return TCP_SRV_ERROR;
    }

    if (tcp_server_listen(server, s_fd) == TCP_SRV_ERROR) {
        goto fail;
    }

    /* create connection pool */
    if (connection_pool_init(server) == TCP_SRV_ERROR) {
        log_error(server->logger, 0, "init connecton pool failed.");
        goto fail;
    }

    return TCP_SRV_OK;

fail:
    close(s_fd);
    return TCP_SRV_ERROR;
}

int tcp_server_create_socket(tcp_server_t *server)
{
    int s_fd, reuse;

    reuse = 1;

    s_fd = socket(AF_INET, SOCK_STREAM, 0); 

    if (s_fd == -1) {
        log_error(server->logger, errno, "create socket failed.");              
        return TCP_SRV_ERROR;
    }

    if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        log_error(server->logger, errno, 
                  "set \"SO_REUSEADDR\" option to socket \"%d\" failed.", s_fd);
        close(s_fd);
        return TCP_SRV_ERROR;
    }

    return server->sock_fd = s_fd;
}

int tcp_server_listen(tcp_server_t *server, int s_fd)
{
    uint_t            port;
    xpe_sockaddr_in  *sa;

    port = server->port;
    sa = &server->sa;

    memset(sa, 0, sizeof(xpe_sockaddr_in));

    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    sa->sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s_fd, (xpe_sockaddr *) sa, sizeof(xpe_sockaddr_in)) == -1) {
        log_error(server->logger, errno, "bind socket \"%d\" failed.", s_fd);
        return TCP_SRV_ERROR;
    }

    if (listen(s_fd, 511) == -1) {
        log_error(server->logger, errno, "listen port \"%d\" failed.", port);
        return TCP_SRV_ERROR;
    }

    return TCP_SRV_OK;
}

int tcp_server_accept(tcp_connection_t *conn)
{
    int               s_fd, c_fd, nodelay;
    u_char           *addr;
    socklen_t         len;
    xpe_sockaddr_in   sa;
    tcp_connection_t *new_conn;

    nodelay = conn->server->nodelay;
    s_fd = conn->conn_fd;
    len = sizeof(xpe_sockaddr_in);

    for (;;) {
        c_fd = accept(s_fd, (xpe_sockaddr *) &sa, &len);
        if (c_fd == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                log_error(conn->logger, errno, 
                          "accept a new connection failed.");
                return TCP_SRV_ERROR;
            } 
        }

        break;
    }

    if (set_nonblock(c_fd) != 0) {
        log_error(conn->logger, 0,
                  "set O_NONBLOCK to connection %d failed.", c_fd);
        close(c_fd);
        return TCP_SRV_ERROR;
    }

    if (nodelay && set_nodelay(c_fd) != 0) {
        log_error(conn->logger, 0,
                  "Set TCP_NODELAY to connection %d failed.", c_fd);
    }

    new_conn = tcp_get_connection(conn->server, c_fd);
    if (new_conn == NULL) {
        log_error(conn->logger, 0,
                  "get connection from pool failed, the pool may be full.");
        close(c_fd);
        return TCP_SRV_ERROR;
    }

    /**
     * add the read event of new tcp connection into event driver 
     */
    if (add_event(conn->server->event_driver, new_conn, EV_READ_EVENT,
              tcp_server_recv) == EVENT_ERROR)
    {
        log_error(conn->logger, 0, 
                  "add the read event of connection %d to event driver failed.",
                  new_conn->conn_fd);
        close(new_conn->conn_fd); 
        tcp_free_connection(conn->server, new_conn);
        return TCP_SRV_ERROR;
    }

    addr = pmalloc(new_conn->pool, 128);
    if (addr == NULL) {
        return TCP_SRV_ERROR;
    }

    x_strcpy(addr, inet_ntoa(sa.sin_addr));
    new_conn->client_addr.data = addr;
    new_conn->client_addr.len = x_strlen(addr);
    new_conn->client_port = ntohs(sa.sin_port);

#ifdef DEBUG
    log_debug(conn->logger, 0, 
              "Accept a new connection(%d) from (addr:%s, port:%d), "
              "and add it to event driver",
              new_conn->conn_fd,
              new_conn->client_addr.data,
              new_conn->client_port);
#endif

    return TCP_SRV_OK;
}

int tcp_server_recv(tcp_connection_t *conn)
{
    int              n, ret;
    buffer_t        *b;
    tcp_request_t   *r;

    r = conn->request; 

    if (r == NULL && (r = tcp_request_init(conn)) == NULL) {
        log_error(conn->logger, 0,
                  "Request from client(addr:%s, port:%d) init failed.",
                  conn->client_addr.data, conn->client_port);
        return TCP_SRV_ERROR;
    }

    b = r->last_buffer;

    n = read(conn->conn_fd, (char *) b->last, b->end - b->last); 
    
    if (n == 0) {
        log_warn(conn->logger, 0, "Client(addr:%s, port:%d) has closed.",
                 conn->client_addr.data, conn->client_port);

        conn->dead_events = EV_READ_EVENT | EV_WRITE_EVENT;
        conn->close = 1;
        return TCP_SRV_OK; 
    }

    if (n == -1) {
        if (errno == EAGAIN) {
            return TCP_SRV_OK;
        } else {
            log_error(conn->logger, 0, 
                      "Connection(%d) is error, will close it.", conn->conn_fd);

            conn->dead_events = EV_READ_EVENT | EV_WRITE_EVENT;
            conn->close = 1;
            return TCP_SRV_ERROR;
        }
    }


    ret = r->parser(r, b->last, b->last + n);
    b->last += n;

    tcp_request_process(r, ret);

    return TCP_SRV_OK;
}

int tcp_server_send(tcp_connection_t *conn)
{
    int            n;
    buffer_t      *buf;
    tcp_request_t *r;
    
    r = conn->request;
    if (r == NULL) {
        return TCP_SRV_OK;
    }

    buf = r->response;

    for (;;) {
        n = write(conn->conn_fd, (char *) buf->buffer,
                  buf->last - buf->buffer);

        if (n == -1) {
            if (errno == EAGAIN) {
                conn->add_events = EV_WRITE_EVENT;                
                return TCP_SRV_OK;
            } else {
                log_error(conn->logger, 0,
                          "Connection(%s, %d) is error, will close it.",
                          conn->client_addr.data, conn->client_port);

                conn->dead_events = EV_READ_EVENT | EV_WRITE_EVENT;
                conn->close = 1;
                r->finish = 1;
                return TCP_SRV_OK;
            }
        }

        buf->buffer += n;

        if (buf->buffer == buf->last) {
            if (conn->events & EV_WRITE_EVENT) {
                conn->dead_events = EV_WRITE_EVENT;                
                conn->write_event_handler = NULL;
            }

            r->finish = 1;
            break;
        }
    }

    return TCP_SRV_OK;
}

int connection_pool_init(tcp_server_t *server)
{
    int               i;
    tcp_connection_t *conn_pool, *conn, *next;

    assert(server->pool != NULL);

    conn_pool = pmalloc(server->pool, 
                        sizeof(tcp_connection_t) * server->connections);
    if (conn_pool == NULL) {
        log_error(server->logger, 0, "pmalloc memory failed.");
        return TCP_SRV_ERROR;
    }

    for (next = NULL, i = server->connections - 1; i >= 0; i--) {
        conn = conn_pool + i;
        conn->next = next;
        next = conn; 
    }

    server->connection_pool = conn_pool;

    return TCP_SRV_OK;
}

tcp_connection_t *tcp_get_connection(tcp_server_t *server, int fd)
{
    mem_pool_t       *pool;
    tcp_connection_t *conn, *conn_pool;

    conn_pool = server->connection_pool;

    if (conn_pool == NULL) {
        return NULL;
    }

    pool = mem_pool_create((u_char *) "connection_pool",
                           CONNECTION_POOL_SIZE, server->logger);
    if (pool == NULL) {
        log_error(server->logger, 0,
                  "create connection's memory pool failed."); 
        return NULL;
    }

    conn = conn_pool;
    conn_pool = conn->next;
    server->connection_pool = conn_pool;

    memset(conn, 0, sizeof(tcp_connection_t));

    conn->conn_fd = fd;
    conn->client_port = -1;
    conn->server = server;
    conn->logger = server->logger;
    conn->pool = pool;

    return conn;
}

void tcp_free_connection(tcp_server_t *server, tcp_connection_t *c)
{
    tcp_request_t *r;

    r = c->request;
    if (r != NULL && r->pool != NULL) {
        mem_pool_destroy(r->pool);
    }

    c->next = server->connection_pool;
    server->connection_pool = c;

    c->conn_fd = -1;
    c->logger = NULL;

    mem_pool_destroy(c->pool);
    c->pool = NULL;
}

tcp_request_t *tcp_request_init(tcp_connection_t *conn)
{
    uint_t           req_buffer_size;
    u_char          *buffer;
    buffer_t        *b, *b1;
    mem_pool_t      *pool;
    tcp_request_t   *r;

    req_buffer_size = conn->server->request_buf_size;
    
    pool = mem_pool_create((u_char *) "request", REQUEST_POOL_SIZE,
                           conn->logger);
    if (pool == NULL) {
        log_error(conn->logger, 0, "Create request memory pool failed.");
        return NULL;
    }

    r = pmalloc(pool, sizeof(tcp_request_t));
    if (r == NULL) {
        return NULL;
    }

    if (req_buffer_size < BUFFER_MIN_SIZE) {
        req_buffer_size = BUFFER_MIN_SIZE;
    } else if (req_buffer_size > BUFFER_MAX_SIZE) {
        req_buffer_size = BUFFER_MAX_SIZE;
    }

    buffer = pmalloc(pool, req_buffer_size + sizeof(buffer_t));
    if (buffer == NULL) {
        return NULL;
    }

    b = (buffer_t *) buffer;

    b->buffer = buffer + sizeof(buffer_t);
    b->end = b->buffer + req_buffer_size;
    b->last = b->buffer;
    b->next = NULL;

    r->buffers = b;
    r->last_buffer = b;

    buffer = pmalloc(pool, req_buffer_size + sizeof(buffer_t));
    if (buffer == NULL) {
        return NULL;
    }

    b1 = (buffer_t *) buffer;

    b1->buffer = buffer + sizeof(buffer_t);
    b1->end = b1->buffer + req_buffer_size;
    b1->last = b1->buffer;
    b1->next = NULL;

    r->response = b1;
    r->last_rep_buf = b1;

    r->conn = conn;
    r->done = 0;
    r->finish = 0;
    r->error = 0;

    r->pool = pool;
    r->logger = conn->logger;

    if (protocol_init(r) == PROTOCOL_ERROR) {
        return NULL;
    }

    conn->request = r;

    return r;
}

int tcp_request_process(tcp_request_t *r, int ret)
{
    u_char   *p;
    buffer_t *buf, *new_buf;

    buf = r->last_buffer;

    if (ret == PROTOCOL_OK) {
        if (buf->last != buf->end) {
            return TCP_SRV_OK;
        }

        p = pmalloc(r->pool, BUFFER_MIN_SIZE + sizeof(buffer_t));
        if (p == NULL) {
            r->error = -1;
            goto response;
        }

        new_buf = (buffer_t *) p;        

        new_buf->buffer = p + sizeof(buffer_t);
        new_buf->last = new_buf->buffer;
        new_buf->end = new_buf->buffer + BUFFER_MIN_SIZE;
        new_buf->next = NULL;

        buf->next = new_buf;

        r->last_buffer = new_buf;

        return TCP_SRV_OK;
    } else if (ret == PROTOCOL_DONE) {
        r->done = 1;

        /**
         * TODO 激发对应的队列线程去持久化
         */
#ifdef DEBUG
        u_char old;

        log_debug(r->logger, 0, "Type: %d", r->protocol->type);

        if (r->protocol->headers_start != NULL 
                && r->protocol->headers_end != NULL)
        { 
            old = *(r->protocol->headers_end);
            *(r->protocol->headers_end) = '\0';
            log_debug(r->logger, 0, "Headers: {%s}", 
                      (u_char *) r->protocol->headers_start);
            *(r->protocol->headers_end) = old;
        } else {
            log_debug(r->logger, 0, "Headers: {None}");
        }

        log_debug(r->logger, 0, "Data len: %d", r->protocol->data_len);

        if (r->protocol->data_start != NULL && r->protocol->data_end != NULL) {
            old = *(r->protocol->data_end);
            *(r->protocol->data_end) = '\0';
            log_debug(r->logger, 0, "Data: %s", 
                      (u_char *) r->protocol->data_start);
            *(r->protocol->data_end) = old;
        } else {
            log_debug(r->logger, 0, "Data: None");
        }
#endif

    } else {
        r->error = ret;
        
        if (r->error != -1) {
            log_error(r->logger, 0, "Request is error(%s)",
                      protocol_err_str(r->error));
        } else {
            log_error(r->logger, 0, "Process request failed.");
        }
    } 


response:
    
    tcp_request_finish(r);

    return TCP_SRV_OK;
}

int tcp_request_finish(tcp_request_t *r)
{
    tcp_connection_t *conn;

    conn = r->conn;

    tcp_respone_generate(r);
   
    tcp_server_send(conn);

    if (r->finish) {
        mem_pool_destroy(r->pool);
        conn->request = NULL;
    }

    return TCP_SRV_OK;
}

void tcp_respone_generate(tcp_request_t *r)
{
    buffer_t *buf;

    buf = r->response;

    switch (r->protocol->type) {
    case PUT_T:
        if (r->error) {
            sprintf((char *) buf->last, "error\r\n");
            buf->last += 7;
        } else {
            sprintf((char *) buf->last, "ok\r\n");
            buf->last += 4;
        }
    case GET_T:
        break;
    case QUEUE_T:
        break;
    case LIST_T:
        break;
    }
}

/**
 * error: return errno
 * success: return 0 
 */
int set_nonblock(int fd)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return errno;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return errno;
    }

    return 0;
}

/**
 * error: return errno
 * success: return 0 
 */
int set_nodelay(int fd)
{
    int nodelay = 1;    

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int)) == -1)
    {
        return errno;
    }

    return 0;
}

