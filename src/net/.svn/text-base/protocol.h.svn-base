/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "config.h"
#include "system.h"

#define PROTOCOL_OK     0
#define PROTOCOL_ERROR -1
#define PROTOCOL_DONE  -2


#define PUT     "PUT"
#define GET     "GET"
#define QUEUE   "QUEUE"
#define LIST    "LIST"

#define UNKNOW  0
#define PUT_T   1
#define GET_T   2
#define QUEUE_T 3
#define LIST_T  4

#define MAX_HEADERS_LEN  1024

extern string_t protocol_err_info[];

typedef int (*protocol_parse_fp) (tcp_request_t *r, u_char *start,
        u_char *end);

struct protocol_s {
    int     type;
    int     state;

    u_char *start;
    u_char *end;

    u_char *headers_start;
    u_char *headers_end;

    size_t  tmp_data_len;
    size_t  data_len;
    void   *data_start_buf;
    u_char *data_start;
    u_char *data_end;
};

int protocol_init(tcp_request_t *r);
int protocol_parse(tcp_request_t *r, u_char *start, u_char *end);

#define protocol_err_str(e)  protocol_err_info[e].data

#endif /* __PROTOCOL_H__ */
