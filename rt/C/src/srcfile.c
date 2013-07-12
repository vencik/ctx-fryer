/**
 *  \brief  Souce code file resource
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/07/03
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2012 Vaclav Krpec
 *
 *  CTX Fryer C library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "srcfile.h"

#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>


/** Source file page size */
#define srcfile_pagesize ((size_t)sysconf(_SC_PAGESIZE))


/*
 * Static functions prototypes
 */

inline static int srcfile_get_page(srcfile_t *srcf, char **page, size_t *size);

static void srcfile_free_page(void *obj, char *page, size_t size);

inline static int srcfile_get_block(srcfile_t *srcf, char **data, size_t *size);

static void srcfile_free_block(void *obj, char *data, size_t size);

inline static int srcfile_get_line(srcfile_t *srcf, char **line, size_t *length);


/*
 * Interface definition
 */

size_t srcfile_page_size(void) {
    return srcfile_pagesize;
}


void srcfile_create(srcfile_t *srcf, const char *filename, srcfile_segmode_t segmode, ...) {
    assert(NULL != srcf);
    assert(NULL != filename);

    memset(srcf, 0, sizeof(srcfile_t));

    /* Copy file name */
    strncpy(srcf->name, filename, sizeof(srcf->name));
    assert('\0' == srcf->name[sizeof(srcf->name) - 1]);
    srcf->name[sizeof(srcf->name) - 1] = '\0';  /* if assert is empty */

    srcf->segmode = segmode;
    srcf->fd      = -1;
    srcf->file    = NULL;

    va_list arg_list;
    va_start(arg_list, segmode);

    switch (segmode) {
        /* Paged mode (memory-mapped) */
        case SRCFILE_SEG_PAGE:

            /* One argument: page size multiplier */
            srcf->page_size = va_arg(arg_list, size_t);
            assert(srcf->page_size > 0);
            srcf->page_size *= srcfile_pagesize;

            break;

        /* Block mode */
        case SRCFILE_SEG_BLOCK:

            /* One argument: the block size */
            srcf->block_size = va_arg(arg_list, size_t);
            assert(srcf->block_size > 0);

            break;

        /* Line mode */
        case SRCFILE_SEG_LINE:

            /* No arguments */

            break;
    }

    va_end(arg_list);
}


int srcfile_open(srcfile_t *srcf) {
    assert(NULL != srcf);
    assert(-1 == srcf->fd);

    srcf->fd = open(srcf->name, O_RDONLY);

    if (-1 == srcf->fd) return errno;

    /* In line mode, we need FILE-style access */
    if (SRCFILE_SEG_LINE == srcf->segmode) {
        srcf->file = fdopen(srcf->fd, "r");

        if (NULL == srcf->file) return errno;
    }

    /* Get file info */
    if (fstat(srcf->fd, &srcf->info)) return errno;

    return 0;
}


int srcfile_close(srcfile_t *srcf) {
    assert(NULL != srcf);

    if (-1 == srcf->fd) return 0;

    if (srcf->file) {
        if (fclose(srcf->file)) return errno;
    }
    else {
        if (close(srcf->fd)) return errno;
    }

    srcf->fd = -1;

    return 0;
}


void srcfile_destroy(srcfile_t *srcf) {
    assert(NULL != srcf);

    srcfile_close(srcf);
}


int srcfile_next_buffer(srcfile_t *srcf, buffer_t **buffer, int *last) {
    assert(NULL != srcf);
    assert(NULL != buffer);

    /* No more data */
    if (srcf->pos >= srcf->info.st_size) return ENODATA;

    buffer_cleanup_fn *cleanup_fn;

    char   *data;
    size_t  size;
    int     erno;

    switch (srcf->segmode) {
        /* Paged mode (memory-mapped) */
        case SRCFILE_SEG_PAGE:

            erno = srcfile_get_page(srcf, &data, &size);

            cleanup_fn = &srcfile_free_page;

            break;

        /* Block mode */
        case SRCFILE_SEG_BLOCK:

            erno = srcfile_get_block(srcf, &data, &size);

            cleanup_fn = &srcfile_free_block;

            break;

        /* Line mode */
        case SRCFILE_SEG_LINE:

            erno = srcfile_get_line(srcf, &data, &size);

            cleanup_fn = &srcfile_free_block;

            break;

        /* Unsupported mode */
        default:
            return EINVAL;
    }

    if (erno) return erno;

    size_t new_pos = srcf->pos + size;
    int    is_last = new_pos >= srcf->info.st_size;

    *buffer = buffer_create(data, size, cleanup_fn, NULL, is_last);

    if (NULL == *buffer) return ENOMEM;

    srcf->pos = new_pos;

    if (NULL != last) *last = is_last;

    return 0;
}


/*
 * Static functions definitions
 */

/**
 *  \brief  Get source file page
 *
 *  The function provides read-only file page mapped to memory
 *  using \c mmap.
 *
 *  \param  srcf  Source file
 *  \param  page  Source file page
 *  \param  size  Source file page size
 *
 *  \retval 0       on success
 *  \retval errno   if \c mmap fails
 */
inline static int srcfile_get_page(srcfile_t *srcf, char **page, size_t *size) {
    assert(NULL != srcf);
    assert(NULL != page);
    assert(NULL != size);

    /* Page size */
    *size = srcf->info.st_size - srcf->pos;
    if (*size > srcf->page_size)
        *size = srcf->page_size;

    /* Map the page to memory */
    *page = (char *)mmap(NULL, *size, PROT_READ,
                         MAP_PRIVATE | MAP_NORESERVE,
                         srcf->fd, srcf->pos);

    return MAP_FAILED == *page ? errno : 0;
}


/**
 *  \brief  Memory-mapped page cleanup callback
 *
 *  \param  obj   User-specified object (not used)
 *  \param  data  Page mapped to memory
 *  \param  size  Page size
 */
static void srcfile_free_page(void *obj, char *page, size_t size) {
    munmap(page, size);
}


/**
 *  \brief  Get data block from source file
 *
 *  The function provides block of source file.
 *  The data is read using \c pread.
 *  Max. size of data block read at once is specified on creation
 *  of the \c srcf object.
 *
 *  \param  srcf  Source file
 *  \param  data  Source block
 *  \param  size  Source block size
 *
 *  \retval 0       on success
 *  \retval ENOMEM  on memory error
 *  \retval errno   if \c read fails
 */
inline static int srcfile_get_block(srcfile_t *srcf, char **data, size_t *size) {
    assert(NULL != srcf);
    assert(NULL != data);
    assert(NULL != size);

    /* Block size */
    *size = srcf->info.st_size - srcf->pos;
    if (*size > srcf->block_size)
        *size = srcf->block_size;

    /* Allocate the data block */
    *data = (char *)malloc(*size * sizeof(char));

    if (NULL == *data) return ENOMEM;

    /* Read the data from file */
    *size = pread(srcf->fd, *data, *size, srcf->pos);

    if (-1 == *size) {
        int erno = errno;
        free(*data);
        return erno;
    }

    return 0;
}


/**
 *  \brief  Dynamic data block cleanup callback
 *
 *  \param  obj   User-specified object (not used)
 *  \param  data  Block of data (dynamicaly allocated)
 *  \param  size  Block size (irrelevant)
 */
static void srcfile_free_block(void *obj, char *data, size_t size) {
    free(data);
}


/**
 *  \brief  Get line from source file
 *
 *  The function reads another line of source from the file.
 *  \c getline is used to recover the line and resolve its end.
 *
 *  \param  srcf    Source file
 *  \param  line    Source line
 *  \param  length  Source line length
 *
 *  \retval 0       on success
 *  \retval errno   if \c fseek or \c getline fails
 */
inline static int srcfile_get_line(srcfile_t *srcf, char **line, size_t *length) {
    assert(NULL != srcf);
    assert(NULL != line);
    assert(NULL != length);

    /*
     * Make sure the position is correct
     * Note that this isn't necessary unless
     * the position is changed otherwise than
     * by getline itself
     */
    if (fseek(srcf->file, srcf->pos, SEEK_SET)) return errno;

    /* Read another line from source file */
    *line = NULL;

    ssize_t rlength = getline(line, length, srcf->file);

    if (-1 == rlength) return errno;

    assert(0 <= rlength);

    /*
     * Note that *length is size of allocated buffer from getline, here
     * (which may and does differ from line length)
     */
    *length = (size_t)rlength;

    return 0;
}
