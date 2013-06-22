#ifndef CTXFryer__srcfile_h
#define CTXFryer__srcfile_h

/**
 *  \brief  Source file
 *
 *  The module provides abstraction of work with source files.
 *
 *  Contents of a source file may be accessed in 3 ways via
 *  this module:
 *  1/ by mapping pages to memory
 *  2/ by reading blocks of octets
 *  3/ by reading ended lines
 *
 *  The user may freely choose between these strategies; the data
 *  is always produced via the buffer interface, the only difference
 *  is in the source file segmentation: to pages, to blocks or to
 *  lines.
 *
 *  In the 1st case, every buffer provides access to a page that is
 *  mapped to memory.
 *  The page size is an integral multiple of system page size
 *  (which is typically 4 KiB).
 *  This approach is probably the fastest way of reading the source
 *  file contents, however, it might use memory quite inefficiently
 *  in case the source file is small (the page may be heavily underused).
 *  It also effectively disables early disposal of unused buffers,
 *  since the probability of not having a single reference to a lexical
 *  item token drops fast with increasing buffer size.
 *
 *  The 2nd approach is probably most known buffering technique; the term
 *  block is used here to distinguish it from the buffer principle
 *  (since we provide more than one implementation).
 *  It allows to overcome both the space inefficiency for small files
 *  and the inability of early disposal of unused source found in the paging
 *  technique, above.
 *  The user may choose the octet block size at will.
 *  Copying the data to the memory blocks is less efficient than mapping pages
 *  to memory; however, for small files, the overhead should not be
 *  that important since there's not too much data, anyway.
 *
 *  The 3rd kind of segmentation might be interresting for naturally
 *  line-organised source, namely if each line represents syntactically
 *  encapsulated construct and the file is processed incrementally.
 *  (top-to-bottom).
 *  In such cases, each buffered line may only be kept in memory as long as
 *  the line processing takes and the run-time memory consumption may be
 *  really low even for large source files.
 *  Note that, however, this approach is quite inefficient in terms of time
 *  and should therefore only be used if time is not too much of an issue
 *  or the source line processing time is substantial and amortises
 *  the reading time well.
 *  Also note that in circumstances described above, all the techniques
 *  will enable for quite efficient memory usage.
 *  Therefore, this one will probably be used least.
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/07/11
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

#include "buffer.h"

#include <unistd.h>
#include <limits.h>
#include <stdio.h>

#include <sys/stat.h>


typedef struct srcfile srcfile_t;  /**< Source file */


/** Source file segmentation mode */
typedef enum {
    SRCFILE_SEG_PAGE,   /**< Source file segmented to pages  (see \c mmap)    */
    SRCFILE_SEG_BLOCK,  /**< Source file segmented to blocks (see \c pread)   */
    SRCFILE_SEG_LINE,   /**< Source file segmented to lines  (see \c getline) */
} srcfile_segmode_t;  /* end of typedef enum */


/** Source file */
struct srcfile {
    char               name[PATH_MAX];  /**< File name                                 */
    srcfile_segmode_t  segmode;         /**< Segmentation mode                         */
    int                fd;              /**< File descriptor                           */
    size_t             pos;             /**< Current position in the source file       */
    size_t             page_size;       /**< Page size  (for page  segmentation, only) */
    size_t             block_size;      /**< Block size (for block segmentation, only) */
    FILE              *file;            /**< File entry (for line  segmentation, only) */
    struct stat        info;            /**< File info from FS                         */
};  /* end of struct srcfile */


/**
 *  \brief  Source file name getter
 *
 *  \param  srcf  Source file object
 *
 *  \return Source file name (read-only)
 */
#define srcfile_name(srcf) ((const char *)((srcf)->name))


/**
 *  \brief  Source file segmentation mode getter
 *
 *  \param  srcf  Source file object
 *
 *  \return Source file segmentation mode
 */
#define srcfile_segmode(srcf) ((srcf)->segmode)


/**
 *  \brief  Source file page size minimum
 *
 *  Might be useful if paging segmentation mode is used and the caller
 *  needs to know the source file page size in advance.
 *
 *  \return Page size (for multiplier of 1, see \ref srcfile_create)
 */
size_t srcfile_page_size(void);


/**
 *  \brief  Source file constructor
 *
 *  \param  srcf      Source file object
 *  \param  filename  File name (including path, if necessary)
 *  \param  segmode   File segmentation mode
 *  \param  ...       Segmentation-specific arguments
 *
 *  Segmentation-specific arguments:
 *  SRCFILE_SEG_PAGE:
 *      \c size_t \c page_size_multiplier
 *          Pages (except the last) will have size of
 *          \c page_size_multiplier \c * \c page_size
 *          (where \c page_size is typically 4 KiB,
 *          call \ref srcfile_page_size if you need to
 *          know the page size in advance).
 *
 *  SRCFILE_SEG_BLOCK:
 *      \c size_t \c max_block_size
 *          Maximal size of data block.
 *          The \c pread routine may provide less than this
 *          and/or the last block may be shorter.
 *
 *  SRCFILE_SEG_LINE:
 *      (none)
 *
 *  \return
 */
void srcfile_create(srcfile_t *srcf, const char *filename, srcfile_segmode_t segmode, ...);


/**
 *  \brief  Open source file
 *
 *  The function MUST be called before source may be read
 *  from the source file.
 *  It opens the file read-only, creates file handler for
 *  line segmentation if necessary and obtains the file
 *  information from the file system (namely its size).
 *
 *  \param  srcf  Source file
 *
 *  \retval 0      on success
 *  \retval errno  on error
 */
int srcfile_open(srcfile_t *srcf);


/**
 *  \brief  Close source file
 *
 *  \param  srcf  Source file
 *
 *  \retval 0      on success
 *  \retval errno  on file closing error
 */
int srcfile_close(srcfile_t *srcf);


/**
 *  \brief  Source file destructor
 *
 *  Finalises and invalidates the object.
 *  Closes the file unless already closed.
 *
 *  \param  srcf  Source file
 */
void srcfile_destroy(srcfile_t *srcf);


/**
 *  \brief  Get next source buffer from the file
 *
 *  The function implements the actual segmentation of the source file.
 *  It provides buffered source and shifts reading position in the file.
 *  End-of-file may be detected either by the last-buffer flag or by
 *  the function return value (errno-compatible).
 *
 *  \param[in]   srcf    Source file
 *  \param[out]  buffer  Source buffer
 *  \param[out]  last    Last bit flag (optional, may be \c NULL)
 *
 *  \retval 0       on success
 *  \retval ENODATA if no more data may be read from the file (aka EOF)
 *  \retval ENOMEM  on memory error (resources exhausted)
 *  \retval errno   on other error (see man page for function used
 *                  for the actual segmentation mode, if necessary)
 */
int srcfile_next_buffer(srcfile_t *srcf, buffer_t **buffer, int *last);

#endif /* end of #ifndef CTXFryer__srcfile_h */
