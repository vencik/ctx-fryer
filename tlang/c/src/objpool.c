/**
 *  \brief  Dynamic object pool
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

#include "objpool.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


void objpool_init(objpool_t *pool, size_t size, size_t cache_max, size_t prealloc_cnt) {
    assert(NULL != pool);

    memset(pool, 0, sizeof(objpool_t));

    pool->size      = size;
    pool->cache_max = cache_max;

    objpool_prealloc(pool, prealloc_cnt);
}


void *objpool_alloc(objpool_t *pool) {
    assert(NULL != pool);

    void *obj = pool->cache;

    /* Cached object available */
    if (NULL != obj) {
        assert(0 < pool->cnt);

        pool->cache = *(void **)obj;

        --pool->cache_cnt;
    }

    /* Allocation is necessary */
    else {
        obj = malloc(pool->size);

        if (NULL != obj)
            ++pool->cnt;
    }

    return obj;
}


void *objpool_calloc(objpool_t *pool) {
    void *obj = objpool_alloc(pool);

    return NULL != obj ? memset(obj, 0, pool->size) : NULL;
}


void objpool_prealloc(objpool_t *pool, size_t cnt) {
    assert(NULL != pool);

    while (cnt--) {
        void *obj = malloc(pool->size);
        if (NULL == obj) break;

        *(void **)obj = pool->cache;
        pool->cache   = obj;

        ++pool->cache_cnt;
        ++pool->cnt;
    }
}


void objpool_free(objpool_t *pool, void *obj) {
    assert(NULL != pool);
    assert(NULL != obj);

    /* Cache the object */
    if (pool->cache_cnt < pool->cache_max) {
        *(void **)obj = pool->cache;
        pool->cache   = obj;

        ++pool->cache_cnt;
    }

    /* Deallocate the object (cache full) */
    else {
        assert(0 < pool->cnt);

        --pool->cnt;

        free(obj);
    }
}


void objpool_cleanup(objpool_t *pool) {
    assert(NULL != pool);

    /* Free cached dynamic object memory */
    while (NULL != pool->cache) {
        void *obj = pool->cache;

        pool->cache = *(void **)obj;

        assert(0 < pool->cnt);

        --pool->cnt;
        --pool->cache_cnt;

        free(obj);
    }
}


void objpool_finalise(objpool_t *pool) {
    assert(NULL != pool);

    objpool_cleanup(pool);

    /* Memory leaks/sanity checks */
    assert(0 == pool->cnt);
    assert(0 == pool->cache_cnt);
}
