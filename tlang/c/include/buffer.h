#ifndef CTXFryer__buffer_h
#define CTXFryer__buffer_h

/**
 *  \brief  Octet buffer
 *
 *  The buffer is shared and uses reference counting mechanism
 *  for determination of its life span.
 *  Any entity (either code or data) that uses the buffer
 *  must reference it (if it's shared).
 *  If the buffer is no longer used by the entity, it must
 *  unreference it and from that point further, it must not
 *  use it.
 *
 *  Buffer data is a continuous block of bytes provided by user.
 *  Buffer data may have arbitrary length; the source is expected
 *  to be segmented to sequence of one or more such buffers.
 *  Lexical items may span over multiple buffers; there are
 *  no special requirements imposed on the source buffering.
 *
 *  The user may also provide cleanup routine that is executed
 *  when all references are lost.
 *  Unless the routine is provided, the data isn't destroyed.
 *
 *  The cleanup routine may also be used as notfication hook
 *  in case the user wishes to use the same memory over and over,
 *  again.
 *  In this case, however, the user must be aware that the data
 *  mustn't be modified before all references to the buffer
 *  are lost (i.e. before the cleanup routine is called).
 *  Otherwise, lexical items tokens may change as they are
 *  described by offset and length in the buffer data
 *  and not copied unless it's required because they span
 *  over more than one buffer.
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/06/29
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

#include <unistd.h>


typedef struct buffer buffer_t;  /**< Buffer */

/**
 *  \brief  Buffer data cleanup routine prototype
 *
 *  \param  obj   User object (if supplied)
 *  \param  data  Buffer data
 *  \param  size  Buffer data size
 */
typedef void buffer_cleanup_fn(void *obj, char *data, size_t size);


/** Buffer */
struct buffer {
    char              *data;        /**< Buffer data                  */
    size_t             size;        /**< Buffer data size             */
    buffer_cleanup_fn *cleanup_fn;  /**< Buffer data cleanup routine  */
    void              *user_obj;    /**< User object (optional)       */
    int                is_last;     /**< Buffer is last in sequence   */
    unsigned int       ref_cnt;     /**< Reference counter            */
    struct buffer     *prev;        /**< Previous buffer              */
    struct buffer     *next;        /**< Next buffer                  */
};  /* end of struct buffer */


/**
 *  \brief  Buffer data getter
 *
 *  \param  buffer  Buffer
 *
 *  \return Buffer data
 */
#define buffer_data(buffer) ((buffer)->data)


/**
 *  \brief  Buffer size getter
 *
 *  \param  buffer  Buffer
 *
 *  \return Buffer data size
 */
#define buffer_size(buffer) ((buffer)->size)


/**
 *  \brief  Buffer user-specified object
 *
 *  \param  buffer  Buffer
 *
 *  \return Buffer user-specified object
 */
#define buffer_user_obj(buffer) ((buffer)->user_obj)


/**
 *  \brief  Reference buffer
 *
 *  The macro increments the buffer reference counter.
 *
 *  \param  buffer  Buffer
 *
 *  \return Buffer reference counter after the operation
 */
#define buffer_ref(buffer) (++(buffer)->ref_cnt)


/**
 *  \brief  Unreference buffer
 *
 *  The macro decrements the buffer reference counter.
 *  If the reference count falls to 0, buffer destructor
 *  is called and the argument is set to \c NULL.
 *
 *  \param  buffer  Buffer
 *
 *  \return Buffer reference counter after the operation
 */
#define buffer_unref(buffer) \
    (--(buffer)->ref_cnt ? : buffer_destroy(buffer), (buffer) = NULL, 0)


/**
 *  \brief  Previous buffer getter
 *
 *  \param  buffer  Buffer
 *
 *  \return Previous buffer
 */
#define buffer_get_prev(buffer) ((buffer)->prev)


/**
 *  \brief  Previous buffer setter
 *
 *  \param  buffer  Buffer
 *  \param  preb    Buffer (shall be set as the previous one)
 */
#define buffer_set_prev(buffer, preb) ((buffer)->prev = (preb))


/**
 *  \brief  Next buffer getter
 *
 *  \param  buffer  Buffer
 *
 *  \return Next buffer (lvalue)
 */
#define buffer_get_next(buffer) ((buffer)->next)


/**
 *  \brief  Next buffer setter
 *
 *  \param  buffer  Buffer
 *  \param  nexb    Buffer (shall be set as the next one)
 */
#define buffer_set_next(buffer, nexb) ((buffer)->next = (nexb))


/**
 *  \brief  Last buffer
 *
 *  \param  buffer  Buffer
 *
 *  \return Last buffer flag (lvalue)
 */
#define buffer_is_last(buffer) ((buffer)->is_last)


/**
 *  \brief  Buffer constructor
 *
 *  \param  data        Buffer data
 *  \param  size        Buffer data size
 *  \param  cleanup_fn  Buffer data cleanup routine (optional)
 *  \param  user_obj    Buffer user-specified object (optional)
 *  \param  is_last     Buffer is the last one in the sequence (non-zero value)
 *
 *  \return Buffer if successfully created, \c NULL in case of memory error
 */
buffer_t *buffer_create(char *data, size_t size, buffer_cleanup_fn *cleanup_fn, void *user_obj, int is_last);


/**
 *  \brief  Buffer destructor
 *
 *  The function destroys the buffer.
 *  If specified, the cleanup routine is called on the buffer data.
 *
 *  Note that you should use the reference counting mechanism
 *  if the buffer is shared; in that case, don't call the destructor,
 *  yourself.
 *  The \ref buffer_unref macro will call it as soon as the reference
 *  count falls to 0.
 *
 *  \param  buffer  Buffer
 */
void buffer_destroy(buffer_t *buffer);

#endif /* end of #ifndef CTXFryer__buffer_h */
