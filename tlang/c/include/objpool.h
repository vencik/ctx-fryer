#ifndef CTXFryer__objpool_h
#define CTXFryer__objpool_h

/**
 *  \brief  Object pool
 *
 *  A simple dynamic object memory cache.
 *
 *  Handy when there's need to create & destroy large amount
 *  of dynamic objects during some period of time while the
 *  actual amount of living objects fluctuates around a level.
 *  In these cases, it's wise to cache objects that would otherwise
 *  be de-allocated only to find out that another object allocation
 *  is required, momentarily.
 *  The cache can ammortise the allocation/deallocation costs
 *  quite nicely in these situations.
 *
 *  Also note that the system allocator is a bottleneck in multi-
 *  threaded environment.
 *  By factorising the dynamic memory usage by object type (size)
 *  and separately caching each class objects using such a pool,
 *  one can dramatically decrease waiting time on the system
 *  allocator in environment where frequent dynamic objects
 *  creation/destruction is done from many threads, concurrently.
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

#include <stddef.h>


/** Object pool */
struct objpool {
    size_t  size;       /**< Object size                */
    size_t  cnt;        /**< All existing objects count */
    void   *cache;      /**< Objects cache              */
    size_t  cache_cnt;  /**< Cached objects count       */
    size_t  cache_max;  /**< Cached objects max. count  */
};  /* end of struct objpool */

typedef struct objpool objpool_t;  /**< Object pool */


/**
 *  \brief  Object pool initialiser (for static pools)
 *
 *  \param  size       Object size
 *  \param  cache_max  Max. amount of cached objects
 */
#define OBJPOOL_INIT(size, cache_max) { \
    (size) < sizeof(void *) ? sizeof(void *) : (size), \
    0, \
    NULL, \
    0, \
    (cache_max), \
}


/**
 *  \brief  Object size
 *
 *  \param  pool  Object pool
 *
 *  \return Pooled object size
 */
#define objpool_size(pool) ((pool)->size)


/**
 *  \brief  Cache max. capacity
 *
 *  \param  pool  Object pool
 *
 *  \return Max. amount of cached objects allowed
 */
#define objpool_cache_max(pool) ((pool)->cache_max)


/**
 *  \brief  Object count
 *
 *  \param  pool  Object pool
 *
 *  \return Number of all objects belonging to the pool
 */
#define objpool_cnt(pool) ((pool)->cnt)


/**
 *  \brief  Cached object count
 *
 *  \param  pool  Object pool
 *
 *  \return Number of currently cached objects
 */
#define objpool_cache_cnt(pool) ((pool)->cache_cnt)


/**
 *  \brief  Allocated object count
 *
 *  \param  pool  Object pool
 *
 *  \return Number of objects that are being actively used
 */
#define objpool_alloc_cnt(pool) ((pool)->cnt - (pool)->cache_cnt)


/**
 *  \brief  Initialise object pool
 *
 *  The function acts like the \ref OBJPOOL_INIT initialiser
 *  and is provided for non-static object pools.
 *  Note that it also allows for preallocation of object
 *  memory blocks (see \ref objpool_prealloc).
 *
 *  \param  pool          Object pool memory (uninitialised)
 *  \param  size          Object size
 *  \param  cache_max     Max. amount of cached objects
 *  \param  prealloc_cnt  Amount of pre-allocated objects
 *                        (passed to \ref objpool_prealloc)
 */
void objpool_init(objpool_t *pool, size_t size, size_t cache_max, size_t prealloc_cnt);


/**
 *  \brief  Allocate an object from the pool
 *
 *  If there's an available object memory cached,
 *  the function provides it to the caller.
 *  Otherwise, it uses the system allocator for allocation
 *  of a new object.
 *  Note that the provided memory isn't initialised.
 *
 *  \param  pool  Object pool
 *
 *  \return Dynamic memory for the object or \c NULL in case of memory error
 */
void *objpool_alloc(objpool_t *pool);


/**
 *  \brief  Allocate initialised object from the pool
 *
 *  The function uses \ref objpool_alloc function to allocate
 *  object memory and initialises it with zeros.
 *
 *  \param  pool  Object pool
 *
 *  \return Initialised dynamic memory for the object or \c NULL
 */
void *objpool_calloc(objpool_t *pool);


/**
 *  \brief  Preallocate & cache objects
 *
 *  The function allocates memory for cached objects.
 *  Sometimes, it may be wise to allocate the memory
 *  in advance so that it's readily available at time
 *  of \ref objpool_alloc call.
 *
 *  Note that the \c cnt parameter is arbitrary and
 *  is NOT compared with the \c cache_max specification.
 *  Also note that if there's memory error during
 *  the preallocation, the function returns, pre-maturely.
 *  You may want to use the \ref objpool_cache_cnt macro
 *  to check if all requeset objects were pre-allocated.
 *
 *  \param  pool  Object pool
 *  \param  cnt   Amount of objects to be pre-allocated
 */
void objpool_prealloc(objpool_t *pool, size_t cnt);


/**
 *  \brief  Return an object back to pool
 *
 *  Unless full, the function returns the object memory to cache.
 *  otherwise it returns it to the system allocator.
 *  Note that the object memory had to be acquired from the pool
 *  (i.e. the call must be paired to \ref objpool_alloc call).
 *
 *  \param  pool  Object pool
 *  \param  obj   Object memory
 */
void objpool_free(objpool_t *pool, void *obj);


/**
 *  \brief  Clean object pool up
 *
 *  The function frees all cached object memory
 *  (so no objects are cached after the call).
 *
 *  \param  pool  Object pool
 */
void objpool_cleanup(objpool_t *pool);


/**
 *  \brief  Finalise object pool
 *
 *  The function should be executed at end of the pool life.
 *  It frees remaining cached object memory using \ref objpool_cleanup
 *  and performs memory leak/sanity checks.
 *  After the call, the pool memory may be invalidated.
 *
 *  Note that when finalising the pool, all objects should
 *  be freed.
 *  Otherwise, the function will cause the process abortion
 *  on logical error (memory leak).
 *
 *  \param  pool  Object pool
 */
void objpool_finalise(objpool_t *pool);

#endif /* end of #ifndef CTXFryer__objpool_h */
