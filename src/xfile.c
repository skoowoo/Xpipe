/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"


file_t *file_open(mem_pool_t *pool, const char *name)
{
    file_t *file;

    file = pmalloc(pool, sizeof(file_t));
    if (file == NULL) {
        return NULL;
    }

    file->name.data = (u_char *) name;
    file->name.len = strlen(name);
    file->fd = open_file_fd(name, FL_RDONLY, 0644);

    if (file->fd == FL_INVALID_FD) {
        return NULL;    
    }

    return file;
}

ssize_t file_read(file_t *f, u_char *buf, size_t size, off_t offset)
{
    ssize_t  n;

    if (offset != FL_DEFAULT_OFFSET && offset != f->offset) {
        if (lseek(f->fd, offset, SEEK_SET) == -1) {
            return XPE_ERROR;
        }

        f->offset = offset;
    }

    n = read(f->fd, buf, size);
    if (n == -1) {
        return XPE_ERROR;
    }

    f->offset += n;

    return n;
}

ssize_t file_write(file_t *f, u_char *buf, size_t size, off_t offset)
{
    ssize_t  n, written;

    written = 0;

    if (offset != FL_DEFAULT_OFFSET && offset != f->offset) {
        if (lseek(f->fd, offset, SEEK_SET) == -1) {
            return XPE_ERROR;
        }

        f->offset = offset;
    }

    for ( ;; ) {
        n = write(f->fd, buf + written, size);
        if (n == -1) {
            return XPE_ERROR;
        }

        f->offset += n;
        written += n;

        if ((size_t) n == size) {
            return written;
        }

        size -= n;
    }
}

