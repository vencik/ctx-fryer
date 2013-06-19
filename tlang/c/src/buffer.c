/**
 *  \brief  Buffer
 *
 *  IMPLEMENTATION NOTES:
 *  The bufferring mechanism is created to allow for incremental parsing
 *  and namely to avoid keeping large amounts of data in memory during
 *  parsing (buffers that are no longer referenced are destroyed,
 *  automatically).
 *  For this reason, the \ref buffer_t objects are cached so that
 *  their allocation costs are amortised as much as possible.
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2012/06/29
 */

#include "buffer.h"
#include "objpool.h"

#include <assert.h>
#include <stddef.h>


#define BUFFER_RESERVE_MAX  32  /**< Max. amount of cached buffer objects   */
#define BUFFER_PREALLOC      4  /**< Amount of pre-allocated buffer objects */


/** Buffer object pool */
static objpool_t buffer_pool = OBJPOOL_INIT(sizeof(buffer_t), BUFFER_RESERVE_MAX);


static void init(void)     __attribute__((constructor));
static void finalise(void) __attribute__((destructor));


buffer_t *buffer_create(char *data, size_t size, buffer_cleanup_fn *cleanup_fn, void *user_obj, int is_last) {
    assert(0 == size || NULL != data);

    buffer_t *buffer = (buffer_t *)objpool_alloc(&buffer_pool);

    if (NULL == buffer) return NULL;

    buffer->data       = data;
    buffer->size       = size;
    buffer->cleanup_fn = cleanup_fn;
    buffer->user_obj   = user_obj;
    buffer->is_last    = is_last;
    buffer->ref_cnt    = 1;

    /* When created, the buffer stands alone */
    buffer->prev = buffer->next = NULL;

    return buffer;
}


void buffer_destroy(buffer_t *buffer) {
    assert(NULL != buffer);

    /* Remove buffer from sequence */
    if (NULL != buffer->prev)
        buffer->prev->next = NULL;
    if (NULL != buffer->next)
        buffer->next->prev = NULL;

    if (NULL != buffer->cleanup_fn)
        buffer->cleanup_fn(buffer->user_obj, buffer->data, buffer->size);

    objpool_free(&buffer_pool, buffer);
}


/**
 *  \brief  Module constructor (GCC only!)
 *
 *  If compiled with gcc, this shall be executed before \c main
 *  routine is called.
 *  Preallocates a few buffer object in the pool in advance
 *  to speed-up initial buffer provision.
 */
static void init(void) {
    objpool_prealloc(&buffer_pool, BUFFER_PREALLOC);
}


/**
 *  \brief  Module destructor (GCC only!)
 *
 *  If compiled with gcc this shall be executed after \c main
 *  routine scope is left.
 *  Cleans the buffer pool up.
 */
static void finalise(void) {
    objpool_finalise(&buffer_pool);
}
