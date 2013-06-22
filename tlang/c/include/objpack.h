#ifndef CTXFryer__objpack_h
#define CTXFryer__objpack_h

/**
 *  \brief  Object pack
 *
 *  A simple dynamic object vector.
 *
 *  Extends the advantages of the Object pool to arrays of objects.
 *  If it's required to construct arrays or multiple objects at once,
 *  use of the module is againg saving number of (de)allocations
 *  and provides the dynamic memory caching.
 *
 *  Note, however, that physically, each pack has the same a priori
 *  defined capacity.
 *  That yields 2 (dual) problems:
 *  1/ The capacity may not be sufficient for an actual request
 *  2/ The pack wastes memory if the amount of requested objects
 *     is notably lower than the pack capacity
 *  The above problems are solved (mitigated) in the following ways:
 *  1/ Using separate allocations for excessive packs (obj. pool bypass)
 *  2/ The last-used pack (unless exhausted) is checked on request;
 *     if the remaining unused capacity is sufficient to satisfy it,
 *     a new one isn't needed
 *
 *  Each pack has a handle that contains meta-info including reference
 *  counter.
 *  Every entity (both data and flow) that's using the handle must
 *  have a reference to it.
 *  When provided, the handle is referenced for the user (caller).
 *  The pack is returned to the pool (or freed) when all references
 *  are lost.
 *  This means that the user might use the reference counting mechanism
 *  in 2 ways: either use just one reference for all the provided
 *  slots or reference the pack for every object initialised (in it).
 *  The latter (typically) delegates the pack release to destructor
 *  of the last object in the pack.
 *
 *  Note that the user MUST NOT use more object slots than requested,
 *  although the pack might actually have greater capacity.
 *  The extra slots may be reserved for another user, see above.
 *  However, using less slots than requested is allowed (as long
 *  as reference counting increase matches decrease).
 *
 *  Obviously, as soon as the user drops its reference(s) to the pack,
 *  it MUST NOT use it further in any way.
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2013/03/09
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2013 Vaclav Krpec
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

#include "objpool.h"

#include <stddef.h>


typedef struct objpack_pool   objpack_pool_t;    /**< Object pack pool   */
typedef struct objpack_handle objpack_handle_t;  /**< Object pack handle */


/** Object pack pool */
struct objpack_pool {
    size_t            obj_size;    /**< Object size           */
    size_t            pack_cap;    /**< Object pack capacity  */
    objpool_t         pack_cache;  /**< Object pack cache     */
    objpack_handle_t *pack;        /**< Last-used object pack */
};  /* end of struct objpack_pool */


/** Object pack handle */
struct objpack_handle {
    objpack_pool_t *pool;     /**< Object pack pool                        */
    size_t          ref_cnt;  /**< Reference counter                       */
    size_t          obj_cnt;  /**< Amount of provided objects (at head)    */
    char            objs[];   /**< Object pack (of size \c pool->pack_cap) */
};  /* end of struct objpack_handle */


/**
 *  \brief  Object pack pool initialiser (for static pools)
 *
 *  \param  size       Object size
 *  \pack   cap        Object pack capacity
 *  \param  cache_max  Max. amount of cached object packs
 */
#define OBJPACK_POOL_INIT(size, cap, cache_max) { \
    (size), \
    (cap), \
    OBJPOOL_INIT(sizeof(objpack_handle_t) + ((cap) * (size)), (cache_max)), \
    NULL, \
}


/**
 *  \brief  Object size
 *
 *  \param  pool  Object pack pool
 *
 *  \return Object size
 */
#define objpack_obj_size(pool) ((pool)->obj_size)


/**
 *  \brief  Object pack capacity
 *
 *  \param  pool  Object pool
 *
 *  \return Object pack capacity
 */
#define objpack_capacity(pool) ((pool)->pack_cap)


/**
 *  \brief  Object pack cache max. capacity
 *
 *  \param  pool  Object pool
 *
 *  \return Max. amount of cached packs
 */
#define objpack_cache_max(pool) objpool_cache_max(&((pool)->pack_cache))


/**
 *  \brief  Object pack cache count
 *
 *  \param  pool  Object pool
 *
 *  \return Number of currently cached packs
 */
#define objpack_cache_cnt(pool) objpool_cache_cnt(&((pool)->pack_cache))


/**
 *  \brief  Object pack pool getter
 *
 *  \param  pack_h  Object pack handle
 *
 *  \return Object pack pool
 */
#define objpack_pool(pack_h) ((pack_h)->pool)


/**
 *  \brief  Increase object pack reference counter
 *
 *  \param  pack_h  Object pack handle
 *  \param  cnt     Reference count
 *
 *  \return Resulting reference count
 */
#define objpack_ref_by(pack_h, cnt) ((pack_h)->ref_cnt += (cnt))


/**
 *  \brief  Increment object pack reference counter
 *
 *  \param  pack_h  Object pack handle
 *
 *  \return Resulting reference count
 */
#define objpack_ref(pack_h) objpack_ref_by((pack_h), 1)


/**
 *  \brief  Decrement object pack reference counter
 *
 *  \param  pack_h  Object pack handle
 *
 *  \return Resulting reference count
 */
#define objpack_unref(pack_h) objpack_unref_by((pack_h), 1)


/**
 *  \brief  Initialise object pack pool
 *
 *  The function creates new object pack pool.
 *  Note that possibility of pre-allocation of the packs depends
 *  on actual memory usage (and the \c cache_max limit).
 *  You may want to check the amount of pre-allocated packs
 *  via \ref objpack_cache_cnt.
 *
 *  \param  pool          Object pack pool memory (uninitialised)
 *  \param  size          Object size
 *  \param  cap           Object pack capacity
 *  \param  cache_max     Max. amount of cached packs
 *  \param  prealloc_cnt  Amount of pre-allocated packs
 */
void objpack_init(objpack_pool_t *pool, size_t size, size_t cap,
                  size_t cache_max, size_t prealloc_cnt);


/**
 *  \brief  Allocate object pack
 *
 *  The function provides another pack of uninitialised dynamic object memory.
 *  The pack is in fact an array of length at least suitable for \c cnt
 *  objects of size defined in the pool initialisation (see \ref objpool_init).
 *
 *  Note the non-optional \c pack_h output argument.
 *  The user needs to keep the handle in order to be able to decrease
 *  the pack reference counter (see \ref objpack_unref_by and
 *  \ref objpack_unref and the module general documentation).
 *  Otherwise, the pack couldn't be deallocated, yielding a memory leak.
 *
 *  The user (caller) is passed one reference on the function exit.
 *  The user MUST unreference the pack by at least the one reference
 *  to get it deallocated.
 *  If needed, the user may obtain more references by \ref objpack_ref_by
 *  or \ref objpack_ref (typically if all the objects need to reference
 *  the pack on their own).
 *  The user MUST NOT unreference the pack by more than the amount of
 *  references it owns.
 *  The user MUST NOT keep on using a pack which it doesn't reference.
 *
 *  \param  pool    Object pack pool
 *  \param  cnt     Object pack size (i.e. count of objects)
 *  \param  pack_h  Object pack handle
 *
 *  \return Object pack or \c NULL in case of memory error
 */
void *objpack_alloc(objpack_pool_t *pool, size_t cnt, objpack_handle_t **pack_h);


/**
 *  \brief  Allocate initialised object pack
 *
 *  The function is identic to \ref objpack_alloc except that the returned
 *  object memory is zeroed.
 *
 *  \param  pool    Object pack pool
 *  \param  cnt     Object pack size (i.e. count of objects)
 *  \param  pack_h  Object pack handle
 *
 *  \return Object pack or \c NULL in case of memory error
 */
void *objpack_calloc(objpack_pool_t *pool, size_t cnt, objpack_handle_t **pack_h);


/**
 *  \brief  Decrease object pack reference counter
 *
 *  In order to return the object pack back to cache (and/or eventually
 *  deallocate it), its reference counter must fall to 0
 *  (meaning that it's no longer used by any flow nor data entity).
 *
 *  See \ref objpack_alloc and the module general documentation for more
 *  info about the reference counting mechanism.
 *
 *  \param  pack_h  Object pack handle
 *  \param  cnt     Reference count
 *
 *  \return Resulting reference count
 */
size_t objpack_unref_by(objpack_handle_t *pack_h, size_t cnt);


/**
 *  \brief  Clean object pack pool up
 *
 *  The function frees all cached object packs
 *  (so no object memory is cached after the call).
 *
 *  \param  pool  Object pack pool
 */
void objpack_cleanup(objpack_pool_t *pool);


/**
 *  \brief  Finalise object pack pool
 *
 *  The function should be executed at the end of the pool life.
 *  It frees remaining cached object packs using \ref objpack_cleanup
 *  and performs memory leak/sanity checks.
 *  After the call, the pool memory may be invalidated.
 *
 *  Note that when finalising the pool, all the object packs
 *  should be unreferenced.
 *  Otherwise, the function will cause the process abortion
 *  on logical error (memory leak).
 *
 *  \param  pool  Object pack pool
 */
void objpack_finalise(objpack_pool_t *pool);

#endif /* end of #ifndef CTXFryer__objpack_h */
