/* $Id: hdrl_buffer.c,v 1.1 2013-10-23 09:13:56 jtaylor Exp $
 *
 * This file is part of the HDRL
 * Copyright (C) 2013,2014,2015 European Southern Observatory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#if !defined(_XOPEN_SOURCE) || (_XOPEN_SOURCE - 0) < 600
#define _POSIX_C_SOURCE 200112L /* posix_fallocate, ftruncate */
/* posix 2001 not enough for ftruncate on glibc 2.12 (SL6.3) */
#define _XOPEN_SOURCE 600
#endif

#include "hdrl_buffer.h"
#include "hdrl_types.h"
#include "hdrl_utils.h"

#include <cpl.h>
#include <cxlist.h>

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

/*-----------------------------------------------------------------------------
                                   Functions
 -----------------------------------------------------------------------------*/

static int hdrl_fallocate(int fd, off_t offset, off_t len)
{
#ifdef __APPLE__
    /* could be improved via fcntl(fd, F_PREALLOCATE, ...); */
    return ftruncate(fd, offset + len);
#else
    return posix_fallocate(fd, offset, len);
#endif
}

static const size_t hdrl_pool_minsize = 2<<20;

struct _hdrl_buffer_ {
    cx_list * pools;
    cx_list * freelist;
    size_t pool_size;
    size_t total_size;
    size_t malloc_thresh;
};

typedef struct {
    char * base;
    char * free_offset;
    size_t size;
    hdrl_free * destructor;
} hdrl_pool_base;

typedef struct {
    hdrl_pool_base p;
    int fd;
} hdrl_pool_mmap;

typedef struct {
    hdrl_pool_base p;
} hdrl_pool_malloc;

typedef hdrl_pool_base hdrl_pool;

static void hdrl_pool_mmap_delete(void * pool)
{
    if (!pool)
        return;
    hdrl_pool_mmap * p = pool;
    /* truncate temp file to stop writeback of dirty pages */
    if (ftruncate(p->fd, 0)) {}
    munmap(p->p.base, p->p.size);
    close(p->fd);
}

static hdrl_pool * hdrl_pool_mmap_new(size_t pool_size)
{
    hdrl_pool_mmap * pool = cpl_malloc(sizeof(*pool));
    pool_size = CX_MAX(pool_size, hdrl_pool_minsize);
    pool->p.destructor = &hdrl_pool_mmap_delete;

    /* Current working directory */
    char *cwd  = hdrl_get_cwd();
    int cwd_fd = hdrl_get_tempfile(cwd, CPL_TRUE);
    cpl_free(cwd);

    /* Allocate TMPDIR first as it usually is a fast disk */
    int tmp_fd = hdrl_get_tempfile(NULL, CPL_TRUE);
    if (hdrl_fallocate(tmp_fd, 0, pool_size) != 0) {
    	close(tmp_fd);
        if (hdrl_fallocate(cwd_fd, 0, pool_size) != 0) {
            close(cwd_fd);
            cpl_free(pool);
            cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                                  "Allocation of %zu bytes failed", pool_size);
            return NULL;
        } else {
        	pool->fd = cwd_fd;
        }
    } else {
    	pool->fd = tmp_fd;
    }

    pool->p.base = mmap(NULL, pool_size, PROT_READ | PROT_WRITE, MAP_SHARED,
    		            pool->fd, 0);

    if (pool->p.base == MAP_FAILED) {
        close(pool->fd);
        cpl_free(pool);
        cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                              "Allocation of %zu bytes failed", pool_size);
        return NULL;
    }

    pool->p.size = pool_size;
    pool->p.free_offset = pool->p.base;

    cpl_msg_debug(cpl_func, "Creating mmap pool %p of size %zu",
                  (const void*)pool, pool_size);

    return (hdrl_pool*)pool;
}

static void hdrl_pool_malloc_delete(void * pool)
{
    if (!pool)
        return;
    cpl_free(((hdrl_pool*)pool)->base);
}

static hdrl_pool * hdrl_pool_malloc_new(size_t pool_size)
{
    hdrl_pool_mmap * pool = cpl_malloc(sizeof(*pool));
    pool->p.size = CX_MAX(pool_size, hdrl_pool_minsize);
    pool->p.destructor = &hdrl_pool_malloc_delete;

    pool->p.base = cpl_malloc(pool_size);
    pool->p.free_offset = pool->p.base;
    cpl_msg_debug(cpl_func, "Creating malloc pool %p of size %zu",
                  (const void*)pool, pool_size);
    return (hdrl_pool*)pool;
}

static void hdrl_pool_delete(hdrl_pool * pool)
{
    if (!pool) {
        return;
    }

    cpl_msg_debug(cpl_func, "Deleting pool %p", (const void*)pool);
    pool->destructor(pool);
    cpl_free(pool);
}

static size_t hdrl_pool_available(hdrl_pool * p)
{
    return (p->base + p->size - p->free_offset);
}

static char * hdrl_pool_alloc(hdrl_pool * p, size_t n)
{
    if (hdrl_pool_available(p) < n) {
        return NULL;
    }
    char * b = p->free_offset;
    p->free_offset += n;
    cpl_msg_debug(cpl_func, "Allocating %zu from pool of size %zu (%zu)",
                  n, p->size, hdrl_pool_available(p));
    return b;
}

static hdrl_pool * hdrl_pool_get_from_ptr(char * HDRL_UNUSED(p))
{
    return NULL;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Create buffer object
 * @param pool_size  size of each memory pool
 * @return buffer object useable to obtain memory mapped memory
 */
/* ---------------------------------------------------------------------------*/
hdrl_buffer * hdrl_buffer_new(void)
{
    /* TODO pool_size probably does not need to be in api */
    hdrl_buffer * buf = cpl_malloc(sizeof(*buf));
    buf->pools = cx_list_new();
    buf->freelist = cx_list_new();
    buf->pool_size = 128ul * (1ul << 20ul);
    buf->total_size = 0;
    buf->malloc_thresh = 0;

    return buf;
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief set total amount of memory the buffer can malloc
 *
 * @param buf  buffer object
 * @param t    amount of memory in MiB
 *
 * @return old amount in wMiB
 * @note changing threshold will only affect future allocation
 */
/* ---------------------------------------------------------------------------*/
size_t hdrl_buffer_set_malloc_threshold(hdrl_buffer * buf, size_t t)
{
    size_t o = buf->malloc_thresh;
    buf->malloc_thresh = t * (1ul << 20ul);
    return o;
}

void hdrl_buffer_readonly(hdrl_buffer * buf, cpl_boolean ro)
{
    /* TODO page alignment for malloc pool */
    for (cx_list_iterator it = cx_list_begin(buf->pools);
         it != cx_list_end(buf->pools);
         it = cx_list_next(buf->pools, it)) {
        hdrl_pool * pool = cx_list_get(buf->pools, it);
        if (ro) {
            mprotect(pool->base, pool->size, PROT_READ);
        }
        else {
            mprotect(pool->base, pool->size, PROT_READ | PROT_WRITE);
        }
    }
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief Allocate memory block from buffer
 *
 * @param buf  buffer object
 * @param size size of memory to allocate
 *
 * @return writable memory block, or NULL on failure
 */
/* ---------------------------------------------------------------------------*/
char * hdrl_buffer_allocate(hdrl_buffer * buf, size_t size)
{
    hdrl_pool * p = NULL;
    char * m;
    for (cx_list_iterator it = cx_list_begin(buf->freelist);
         it != cx_list_end(buf->freelist);
         it = cx_list_next(buf->freelist, it)) {
       hdrl_pool * _p = cx_list_get(buf->freelist, it);
       if (hdrl_pool_available(_p) >= size) {
           p = _p;
           cpl_msg_debug(cpl_func, "Found free available in pool.");
           break;
       }
    }

    if (p == NULL) {
        /* no free pool, clear freelist (TODO clear full pools earlier) */
        cx_list_empty(buf->freelist);
        if (buf->total_size + size < buf->malloc_thresh ||
            getenv("HDRL_BUFFER_MALLOC")) {
            p = hdrl_pool_malloc_new(CX_MAX(size, buf->pool_size));
        }
        else {
            p = hdrl_pool_mmap_new(CX_MAX(size, buf->pool_size));
        }
        cx_list_push_back(buf->pools, p);
        /* add new pool to freelist if it has free available left */
        if (size < buf->pool_size / 2) {
            cx_list_push_back(buf->freelist, p);
        }
    }

    m = hdrl_pool_alloc(p, size);
    buf->total_size += size;
    return m;
}

void hdrl_buffer_free(hdrl_buffer * HDRL_UNUSED(buf), char * p)
{
    hdrl_pool * pool = hdrl_pool_get_from_ptr(p);
    if (pool) {}
    /* TODO add free, only from top of pool should be simple,
     * MADV_DONTNEED may be enough */
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief delete buffer
 * @param buf buffer
 *
 * Invalidates memory of all objects contained in the buffer
 */
/* ---------------------------------------------------------------------------*/
void hdrl_buffer_delete(hdrl_buffer * buf)
{
    if (!buf) {
        return;
    }

    cpl_msg_debug(cpl_func, "Deleting buffer with %zu pools",
                  cx_list_size(buf->pools));
    cx_list_destroy(buf->pools, (hdrl_free*)&hdrl_pool_delete);
    cx_list_delete(buf->freelist);
    cpl_free(buf);
}
