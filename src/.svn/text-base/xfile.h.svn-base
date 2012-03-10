/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __FILE_H__
#define __FILE_H__

#include "config.h"
#include "system.h"

#define default_log_file    "log/xpipe.log"

#define FL_RDWR       O_RDWR
#define FL_RDONLY     O_RDONLY 
#define FL_WRONLY     O_WRONLY
#define FL_CREATE     O_CREAT
#define FL_APPEND     O_WRONLY|O_APPEND
#define FL_TRUNC      O_CREAT|O_TRUNC
#define FL_NOBLOCK    O_NONBLOCK

#define FL_DEFAULT_OFFSET -1
#define FL_BEGIN_OFFSET 0
#define FL_INVALID_FD -1

struct file_s {
    int         fd;
    off_t       offset;
    string_t    name;
};


#define open_file_fd(name, mode, access) open((const char *)name, mode, access)
#define write_fd(fd, buf, size) write(fd, (const char *) buf, size)
#define file_close(fd)  close(fd)
#define file_delete(name)  unlink((const char*)name)

#define write_stdout(buf, size)  write(1, buf, size)

file_t *file_open(mem_pool_t *pool, const char *name);
ssize_t file_read(file_t *f, u_char *buf, size_t size, off_t offset);
ssize_t file_write(file_t *f, u_char *buf, size_t size, off_t offset);


#endif /* __FILE_H__ */
