/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

#define err_type_invalid        1
#define err_only_cr             2
#define err_type_not_found      3
#define err_headers_invalid     4
#define err_data_len_invalid    5
#define err_needless_data       6


string_t protocol_err_info[] = {
    string_null,
    xstring("Type is invalid."),
    xstring("Only \"\r\", \"\n\" not found."),
    xstring("Type not found."),
    xstring("Headers are invalid."),
    xstring("Data len is invalid."),
    xstring("Data is needless.")
};

int protocol_init(tcp_request_t *r)
{
    protocol_t *pro;

    pro = pcalloc(r->pool, sizeof(protocol_t));
    if (pro == NULL) {
        return PROTOCOL_ERROR;
    }

    r->protocol = pro;
    r->parser = protocol_parse;

    return PROTOCOL_OK;
}

int protocol_parse(tcp_request_t *r, u_char *start, u_char *end)
{
    size_t      data_len;
    u_char      c, *p;
    protocol_t *pro;

    pro = r->protocol;

    enum {
        sw_start = 0,
        sw_type,
        sw_type_cr,
        sw_type_lf,
        sw_headers,
        sw_headers_cr,
        sw_headers_lf,
        sw_data_len,
        sw_data_len_cr,
        sw_data_len_lf,
        sw_data
    } state; 

    state = pro->state;
    data_len = pro->tmp_data_len;

    for (p = start; p != end; p++) {
        c = *p;

        switch (state) {
        case sw_start:
            if (is_blank(c)) {
                break;
            }

            if (c < 'A' || c > 'Z') {
                return err_type_invalid;
            }

            pro->start = p;
            state = sw_type;

            break;
        case sw_type:
            if (c == CR) {
                pro->end = p;
                state = sw_type_cr;
                break;
            }

            if (c < 'A' || c > 'Z') {
                return err_type_invalid;
            }

            break;
        case sw_type_cr:
            if (c != LF) {
                return err_only_cr;
            }

            switch (pro->end - pro->start) {
            case 3:
                if (x_strncmp(pro->start, PUT, 3) == 0) {
                    pro->type = PUT_T;
                    break;
                }

                if (x_strncmp(pro->start, GET, 3) == 0) {
                    pro->type = GET_T;
                    break;
                }

                return err_type_not_found;
            case 4:
                if (x_strncmp(pro->start, LIST, 4) == 0) {
                    pro->type = LIST_T;
                    break;
                }

                return err_type_not_found;
            case 5:
                if (x_strncmp(pro->start, QUEUE, 5) == 0) {
                    pro->type = QUEUE_T;
                    break;
                }

                return err_type_not_found;
            default:
                return err_type_not_found;
            }

            state = sw_type_lf;
            break;
        case sw_type_lf:
            if (c == CR) {
                state = sw_headers_cr;
                break;
            }

            if (!is_letter(c) && !is_digit(c) 
                    && c != ';' && c != '_' && c != '=') 
            {
                return err_headers_invalid;
            }

            pro->headers_start = p;
            state = sw_headers;
            break;
        case sw_headers:
            if (c == CR) {
                pro->headers_end = p;
                state = sw_headers_cr;
                break;
            }

            if (!is_letter(c) && !is_digit(c) 
                    && c != ';' && c != '_' && c != '=') 
            {
                return err_headers_invalid;
            }

            break;
        case sw_headers_cr:
            if (c != LF) {
                return PROTOCOL_ERROR;
            }

            state = sw_headers_lf;
            break;
        case sw_headers_lf:
            if (pro->type != PUT_T) {
                return err_needless_data;            
            } else {
                if (!is_digit(c)) {
                    return err_data_len_invalid;
                }

                pro->start = p;
                state = sw_data_len;
                break;
            }
        case sw_data_len:
            if (c == CR) {
                pro->end = p;
                state = sw_data_len_cr;
                break;
            }

            if (!is_digit(c)) {
                return err_data_len_invalid;
            }

            break;
        case sw_data_len_cr:
            if (c != LF) {
                return err_only_cr; 
            }
            
            *(p - 1) = '\0';

            data_len = x_atoi(pro->start);
            pro->data_len = data_len;

            *(p - 1) = '\0';

            if (data_len <= 0) {
                return err_data_len_invalid;
            }

            state = sw_data_len_lf;
            break;
        case sw_data_len_lf:
            pro->data_start_buf = r->last_buffer;
            pro->data_start = p;
            data_len--;

            state = sw_data;
            break;
        case sw_data:
            if (data_len-- == 0) {
                return err_needless_data;
            }

            break;
        } 
    }

    if (pro->type != UNKNOW && pro->type != PUT_T && state == sw_headers_lf) {
        return PROTOCOL_DONE;
    }

    if (pro->type == PUT_T && state == sw_data && data_len == 0) {
        pro->data_end = p;
        return PROTOCOL_DONE;
    }

    pro->state = state;
    pro->tmp_data_len = data_len;

    return PROTOCOL_OK;
}
