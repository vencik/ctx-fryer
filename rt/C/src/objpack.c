/**
 *  \brief  Dynamic object pack
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

#include "objpack.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


void objpack_init(objpack_pool_t *pool, size_t size, size_t cap,
                  size_t cache_max, size_t prealloc_cnt)
{
    assert(NULL != pool);

    pool->obj_size = size;
    pool->pack_cap = cap;

    size_t pack_size = sizeof(objpack_handle_t) + size * cap;

    objpool_init(&(pool->pack_cache), pack_size, cache_max, prealloc_cnt);

    pool->pack = NULL;
}


void *objpack_alloc(objpack_pool_t *pool, size_t cnt, objpack_handle_t **pack_h) {
    assert(NULL != pool);
    assert(NULL != pack_h);

    /* More objects than pack capacity requested */
    if (cnt > pool->pack_cap) {
        *pack_h = (objpack_handle_t *)malloc(
                      sizeof(objpack_handle_t) + pool->obj_size * cnt);

        /* Memory exhausted */
        if (NULL == *pack_h) return NULL;

        (*pack_h)->pool    = pool;
        (*pack_h)->ref_cnt = 1;
        (*pack_h)->obj_cnt = cnt;

        return (*pack_h)->objs;
    }

    /* Check last-used pack remaining capacity */
    if (NULL != pool->pack) {
        if (pool->pack_cap - pool->pack->obj_cnt < cnt) {
            objpack_unref(pool->pack);

            pool->pack = NULL;
        }
    }

    /* Get another pack from the cache */
    if (NULL == pool->pack) {
        pool->pack = (objpack_handle_t *)objpool_alloc(&(pool->pack_cache));

        /* Memory exhausted */
        if (NULL == pool->pack) return NULL;

        pool->pack->pool    = pool;
        pool->pack->ref_cnt = 1;
        pool->pack->obj_cnt = 0;
    }

    /* Provide pack */
    assert(NULL != pool->pack);
    assert(0 < pool->pack->ref_cnt);

    ++pool->pack->ref_cnt;

    void *pack = pool->pack->objs + (pool->pack->obj_cnt * pool->obj_size);

    pool->pack->obj_cnt += cnt;

    *pack_h = pool->pack;

    /* Unref. exhausted pack (to enable earliest possible deallocation) */
    if (pool->pack->obj_cnt == pool->pack_cap) {
        --pool->pack->ref_cnt;

        pool->pack = NULL;
    }

    /* Sanity checks */
    assert((*pack_h)->obj_cnt <= pool->pack_cap);

    return pack;
}


void *objpack_calloc(objpack_pool_t *pool, size_t cnt, objpack_handle_t **pack_h) {
    assert(NULL != pool);

    void *pack = objpack_alloc(pool, cnt, pack_h);

    if (NULL != pack)
        memset(pack, 0, cnt * pool->obj_size);

    return pack;
}


size_t objpack_unref_by(objpack_handle_t *pack_h, size_t cnt) {
    assert(NULL != pack_h);
    assert(pack_h->ref_cnt >= cnt);

    size_t ref_cnt = (pack_h->ref_cnt -= cnt);

    if (ref_cnt) return ref_cnt;

    /* Destroy excessive size pack (these are not cached) */
    if (pack_h->obj_cnt > pack_h->pool->pack_cap)
        free(pack_h);

    /* Return pack back to cache */
    else
        objpool_free(&(pack_h->pool->pack_cache), pack_h);

    return 0;
}


void objpack_cleanup(objpack_pool_t *pool) {
    assert(NULL != pool);

    /* Release last-used pack */
    if (NULL != pool->pack) {
        objpack_unref(pool->pack);

        pool->pack = NULL;
    }

    /* Cleanup cache */
    objpool_cleanup(&(pool->pack_cache));
}


void objpack_finalise(objpack_pool_t *pool) {
    assert(NULL != pool);

    objpack_cleanup(pool);
}
