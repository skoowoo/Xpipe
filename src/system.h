/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __HEADERS_H__
#define __HEADERS_H__

#define XPE_OK      0
#define XPE_ERROR   -1

#define LF      (u_char) 10
#define CR      (u_char) 13
#define CRLF    "\x0d\x0a"


typedef unsigned int            uint_t;

typedef struct conf_file_s          conf_file_t;
typedef struct logger_s             logger_t;
typedef struct file_s               file_t;
typedef struct mem_pool_s           mem_pool_t;
typedef struct dynamic_array_s      dynamic_array_t;
typedef struct net_event_driver_s   net_event_driver_t;
typedef struct tcp_request_s        tcp_request_t;
typedef struct tcp_response_s       tcp_response_t;
typedef struct protocol_s           protocol_t;
typedef struct tcp_connection_s     tcp_connection_t;
typedef struct tcp_server_s         tcp_server_t;
typedef struct system_module_s      system_module_t;

typedef struct {
    int          stop;
    int          sys_mod_num;
    mem_pool_t  *conf_pool;
    conf_file_t *conf;
    logger_t    *main_logger;
    logger_t    *log_stdout;
    logger_t    *default_logger;   
} xpipe_resource_t;

typedef int (*event_handler_fp) (tcp_connection_t *conn);

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <execinfo.h>
#include <sys/time.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <signal.h>

#include "xstring.h"
#include "mem_pool.h"
#include "dynamic_array.h"
#include "xfile.h"
#include "times.h"
#include "logger.h"
#include "conf_file.h"
#include "mod_manager.h"

#include "net/protocol.h"
#include "net/tcp_server.h"
#include "net/net_event.h"
#include "net/net_epoll.h"


#endif /* __HEADERS_H__ */
