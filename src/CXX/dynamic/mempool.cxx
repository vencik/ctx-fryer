/**
 *  \brief  Pool (cache) of dynamic memory objects
 *
 *  Implementation.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/12/27
 *
 *  Legal notices
 *
 *  Copyright 2013 Vaclav Krpec
 *
 *  CTX Fryer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "config.hxx"
#include "dynamic/mempool.hxx"

#include <stdexcept>
#include <cassert>


namespace dynamic {

// objpool

void * objpool::obj_unlist(size_t n) {
    void * obj_list = NULL;

    if (n) {
        assert(m_cnt_pool >= n);

        obj_list = m_pool;

        m_cnt_pool -= n;

        void * obj_list_tail;

        do {
            assert(NULL != m_pool);

            obj_list_tail = m_pool;

            m_pool = *((void **)m_pool);

        } while (--n);

        *((void **)obj_list_tail) = NULL;  // terminate obj. list
    }

    return obj_list;
}


void objpool::init(
    size_t size,
    size_t prealloc,
    size_t plimit,
    size_t tlimit) throw(std::logic_error)
{
    // Sanity checks
    assert(NULL == m_pool);
    assert(0 == m_size);
    assert(0 == m_cnt_pool);
    assert(0 == m_cnt_total);

    // Set data members
    m_size   = size;
    m_plimit = plimit;
    m_tlimit = tlimit;

    if (m_size < sizeof(void *))
        m_size = sizeof(void *);

    if (prealloc > m_plimit)
        throw std::logic_error("objpool: pool limit exceeded");

    if (prealloc > m_tlimit)
        throw std::logic_error("objpool: total limit exceeded");

    if (m_plimit > m_tlimit)
        throw std::logic_error("objpool: pool limit exceeds total limit");

    for (; prealloc; --prealloc) {
        void * obj = obj_alloc(m_size);

        ++m_cnt_total;

        obj_enlist(obj);
    }
}


#ifdef HAVE_CXX11
/** Swap with rvalue (no locking done) */
void objpool::swap(objpool && rval) {
    void *        pool      = m_pool;
    const size_t  size      = m_size;
    size_t        cnt_pool  = m_cnt_pool;
    size_t        cnt_total = m_cnt_total;
    size_t        plimit    = m_plimit;
    size_t        tlimit    = m_tlimit;

    m_pool      = rval.m_pool;
    m_size      = rval.m_size;
    m_cnt_pool  = rval.m_cnt_pool;
    m_cnt_total = rval.m_cnt_total;
    m_plimit    = rval.m_plimit;
    m_tlimit    = rval.m_tlimit;

    rval.m_pool      = pool;
    rval.m_size      = size;
    rval.m_cnt_pool  = cnt_pool;
    rval.m_cnt_total = cnt_total;
    rval.m_plimit    = plimit;
    rval.m_tlimit    = tlimit;
}
#endif  // end of #ifdef HAVE_CXX11


void objpool::set_pool_limit(size_t limit) throw(std::logic_error) {
    void * trash = NULL;
    {
        lock4scope(m_mutex);

        if (limit > m_tlimit)
            throw std::logic_error("objpool: new pool limit exceeds total limit");

        m_plimit = limit;

        // Unlist of over-limit objects
        if (m_cnt_pool > m_plimit) {
            size_t diff = m_cnt_pool - m_plimit;

            m_cnt_total -= diff;

            trash = obj_unlist(diff);
        }
    }

    // Destroy over-limit objects (unlocked)
    objlist_free(trash);
}


void objpool::set_total_limit(size_t limit) throw(std::logic_error) {
    void * trash = NULL;

    {
        lock4scope(m_mutex);

        if (limit > m_plimit)
            throw std::logic_error("objpool: pool limit exceeds new total limit");

        if (limit < m_tlimit && m_cnt_total - m_cnt_pool > limit)
            throw std::logic_error("objpool: can't meet new total limit");

        m_tlimit = limit;

        // Unlist of over-limit objects
        if (m_cnt_total > m_tlimit) {
            m_cnt_total = m_tlimit;

            trash = obj_unlist(m_cnt_total - m_tlimit);
        }
    }

    // Destroy over-limit objects (unlocked)
    objlist_free(trash);
}


void * objpool::lim_alloc() {
    size_t size;

    {
        lock4scope(m_mutex);

        if (m_cnt_pool) return obj_unlist();

        if (!(m_cnt_total < m_tlimit)) return NULL;

        size = m_size;
    }

    void * obj = obj_alloc(size);  // unlocked

    lock4scope(m_mutex);

    ++m_cnt_total;

    return obj;
}


inline void * objpool::lim_alloc_impl() {
    void * obj;

    if (m_cnt_pool) {
        obj = obj_unlist();
    }
    else if (m_cnt_total < m_tlimit) {
        size_t size = m_size;

        unlock4scope(m_mutex);

        obj = obj_alloc(size);  // unlocked
    }
    else
        return NULL;  // limit reached

    ++m_cnt_total;

    return obj;
}


void * objpool::lim_alloc(double timeout, bool & busy) {
#ifdef HAVE_MT__MUTEX__TRYLOCK_WITH_TIMEOUT
    // If timed trylock is not supported, the timeout is not considered
    // TBD: is it the best solution?
    // Shouldn't we be more permissive, or (on the other hand)
    // actually strictly forbid the iface in that case?
    if (0.0 == timeout)
#endif
        busy = !m_mutex.trylock();
#ifdef HAVE_MT__MUTEX__TRYLOCK_WITH_TIMEOUT
    else
        busy = !m_mutex.trylock(timeout);
#endif

    if (busy) return NULL;  // lock not acquired within timeout

    void * obj = lim_alloc_impl();

    m_mutex.unlock();

    return obj;
}


void * objpool::lim_alloc(double timeout, double & lock_time, bool & busy) {
#ifdef HAVE_MT__MUTEX__TRYLOCK_WITH_TIMEOUT
    // If timed trylock is not supported, the timeout is not considered
    // TBD: see above
    if (0.0 == timeout) {
#endif
        lock_time = 0.0;

        busy = !m_mutex.trylock();
#ifdef HAVE_MT__MUTEX__TRYLOCK_WITH_TIMEOUT
    }
    else
        busy = !m_mutex.trylock(timeout, lock_time);
#endif

    if (busy) return NULL;  // lock not acquired within timeout

    void * obj = lim_alloc_impl();

    m_mutex.unlock();

    return obj;
}


void objpool::free(void * obj) {
    assert(NULL != obj);

    {
        lock4scope(m_mutex);

        if (m_cnt_pool < m_plimit) {
            obj_enlist(obj);
            return;
        }

        --m_cnt_total;
    }

    obj_free(obj);  // unlocked
}


// ascale_objpool

void ascale_objpool::objpool_entries::remove(
    ascale_objpool::objpool_entry * entry)
{
    assert(NULL != entry);

    if (NULL != entry->prev)
        entry->prev->next = entry->next;
    else {
        assert(entry == m_head);

        m_head = entry->next;
    }

    if (NULL != entry->next)
        entry->next->prev = entry->prev;

    entry->prev = entry->next = NULL;
}


void ascale_objpool::objpool_entries::move_to(
    objpool_entry * entry,
    objpool_entry * pos)
{
    assert(NULL != entry);
    assert(NULL != pos);
    assert(entry != pos);

    // Unlist entry
    if (NULL != entry->prev)
        entry->prev->next = entry->next;
    else {
        assert(entry == m_head);

        m_head = entry->next;
    }

    if (NULL != entry->next)
        entry->next->prev = entry->prev;

    // Enlist entry
    if (NULL != pos->prev)
        pos->prev->next = entry;
    else {
        assert(pos == m_head);

        m_head = entry;
    }

    entry->prev = pos->prev;
    entry->next = pos;

    pos->prev = entry;
}


const size_t ascale_objpool::default_alt_wsize = 200;


ascale_objpool::objpool_entry * ascale_objpool::add_pool(
    size_t prealloc,
    bool in_use)
{
    size_t obj_size = sizeof(obj_header) + m_size;

    objpool_entry * pool_entry = m_objpools.create(
        obj_size, prealloc, m_plimit, m_alt_wsize);  // unlocked

    if (NULL != pool_entry) {
        lock4scope(m_mutex);

        m_objpools.add(pool_entry);  // under lock

        pool_entry->in_use = in_use;
    }

    return pool_entry;
}


void ascale_objpool::rm_pool(objpool_entry * pool_entry) {
    assert(NULL != pool_entry);
    assert(!pool_entry->in_use);

    {
        lock4scope(m_mutex);

        m_objpools.remove(pool_entry);  // under lock
    }

    m_objpools.destroy(pool_entry);  // unlocked
}


void * ascale_objpool::lim_alloc() {
    objpool_entry * pool_entry   = NULL;
    bool            pool_created = false;

    {
        lock4scope(m_mutex);

        if (!(m_objcnt < m_tlimit)) return NULL;

        // Get pool on the pool list head (the one with fastest
        // lock acquisition in average) or create new one if there's none
        // Note that although it might look convenient to pre-alloc
        // at least one node in the new pool, it's not; we want to
        // avoid such costly operations under the global lock
        pool_entry = m_objpools.head();

        if (NULL == pool_entry) {
            unlock4scope(m_mutex);

            pool_entry = add_pool(0, true);

            pool_created = NULL != pool_entry;

            if (!pool_created) return NULL;  // entry pool limit
        }
        else
            pool_entry->in_use = true;

        // Either object will be provided or an exception thrown
        // Note that it is indeed necessary to increment the counter here;
        // otherwise, the total limit might get violated in concurrent
        // environment (see above)
        ++m_objcnt;
    }

    assert(NULL != pool_entry);
    assert(pool_entry->in_use);

    double lock_time;
    bool   busy;
    void * obj;

    try {
        obj = pool_entry->pool_impl.lim_alloc(m_alt_th, lock_time, busy);
    }
    catch (std::bad_alloc & x) {
        // Correct the pre-incremented counter to keep consistency
        lock4scope(m_mutex);

        --m_objcnt;

        throw x;  // it's still bad allocation...
    }

    {
        // Concurrency is high, create new pool for the allocation
        if (busy) {
            // ... but not if we've just created one
            if (!pool_created) {
                // As above, no preallocation is done to avoid costly
                // operations under global lock
                objpool_entry * new_pool_entry = add_pool(0, true);

                lock4scope(m_mutex);

                // Limit reached
                if (NULL == new_pool_entry) {
                    pool_entry->in_use = false;  // not interested any more

                    pool_entry = new_pool_entry;
                }
            }

            // Note that we don't measure memory allocation time.
            // That's something we can't affect in the end, anyway.
            // Instead, we're attempting to detect heavy concurrency.
            // Obviously, there may be no concurrency on a new pool,
            // hence no measurement is necessary, here.
            obj = pool_entry->pool_impl.lim_alloc();

            lock4scope(m_mutex);

            pool_entry->in_use = false;  // done
        }

        // Update pool position in the list accordingly to
        // average lock acquisition time
        else {
            lock4scope(m_mutex);

            // Note that at this point, the pool may no longer
            // reside at the list head, since a parallel operation
            // could have already moved it further
            lock_time = pool_entry->alt_avg.push(lock_time);

            objpool_entry * pos  = pool_entry;
            bool            move = false;

            // Try right neighbours
            for (pos = pos->next; NULL != pos; pos = pos->next) {
                if (lock_time <= pos->alt_avg)
                    break;  // position found

                move = true;  // we shall have to move the pool
            }

            // Try left neighbours (if still necessary)
            if (!move) {
                pos = pool_entry;

                while (NULL != pos) {
                    pos = pos->prev;

                    if (lock_time >= pos->alt_avg) {
                        pos = pos->next;

                        break;  // position found
                    }

                    move = true;
                }
            }

            // Move at due position
            if (move)
                m_objpools.move_to(pool_entry, pos);

            pool_entry->in_use = false;  // done
        }
    }

    assert(NULL != obj);  // we don't impose limit on the mem. obj. pools

    return obj_provide(obj, pool_entry);
}

}  // end of namespace dynamic
