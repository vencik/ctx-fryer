#ifndef dynamic__mempool_hxx
#define dynamic__mempool_hxx

/**
 *  \brief  Pool (cache) of dynamic memory objects
 *
 *  Object pool (\ref objpool) provides fixed size chunks of dynamic memory
 *  and caches them for further use.
 *  It may be usefull in various situations; namely for implementation
 *  of efficient allocators of heavily dynamic node-based data structures
 *  (like trees etc).
 *
 *  The pool may also help to widen allocator locking bottlenecks if very frequent
 *  dynamic memory allocations tend to cause long waiting for global allocator
 *  mutex (because multiple instances may be used without the need for mutual
 *  exclusion when accessing different ones).
 *
 *  Based on the \ref objpool, \ref mempool provides (more) general allocator
 *  of dynamic memory chunks of arbitrary size with caching.
 *  In principle, \c mempool consists of multiple object pools, each providing
 *  space for certain range of sizes.
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
#include "mt/mutex.hxx"
#include "stats/avg.hxx"

#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <vector>
#include <list>


/** \cond */
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif
/** \endcond */


namespace dynamic {

/**
 *  \brief  Dynamic memory object pool
 *
 *  IMPLEMENTATION NOTES:
 *  Each node has minimal size of \c sizeof(void\c *).
 *  Pooled objects are stored in linked list (implemented directly by the
 *  objects).
 *  When the memory is provided to a user, it becomes the user's responsibility
 *  to release it back (to the right pool).
 *  The pool status is checked upon destruction; pooled objects are released
 *  back to the system and if memory leak is detected (i.e. there are still
 *  unreleased objects) an exception is thrown.
 *  The pool however has no control of the objects currently provided, nor
 *  can tell whether a memory chunk that is released back to it by an user
 *  really belongs to it.
 *  Users MUST know their dynamic objects allocator and use it correctly.
 */
class objpool {
    private:

    void *            m_pool;       /**< Pool                            */
    size_t            m_size;       /**< Object size (in bytes)          */
    size_t            m_cnt_pool;   /**< Number of objects in pool       */
    size_t            m_cnt_total;  /**< Total number of objects managed */
    size_t            m_plimit;     /**< Pooled objects limit            */
    size_t            m_tlimit;     /**< Total objects limit             */
    mutable mt::mutex m_mutex;      /**< Operations mutex                */

    /**
     *  \brief  Enlist object (MT unsafe)
     *
     *  \param  obj  Object
     */
    inline void obj_enlist(void * obj) {
        *((void **)obj) = m_pool;
        m_pool = obj;
        ++m_cnt_pool;
    }

    /**
     *  \brief  Unlist object (MT unsafe)
     *
     *  Note that the pool MUST NOT be empty.
     *
     *  \return Pool (former) head object
     */
    inline void * obj_unlist() {
        assert(NULL != m_pool && 0 < m_cnt_pool);

        void * obj = m_pool;
        m_pool = *((void **)obj);
        --m_cnt_pool;

        return obj;
    }

    /**
     *  \brief  Unlist \n objects (MT unsafe)
     *
     *  Note that the pool MUST contain enough objects.
     *
     *  \param  n  Object count
     *
     *  \return Object list (\c NULL if n == 0)
     */
    void * obj_unlist(size_t n);

    /**
     *  \brief  Allocate another object
     *
     *  \param  size  Object size
     *
     *  \return Object
     */
    static void * obj_alloc(size_t size) throw(std::bad_alloc) {
        void * obj = ::malloc(size);

        if (NULL == obj)
            throw std::bad_alloc();

        return obj;
    }

    /**
     *  \brief  Free object
     *
     *  param  obj  Object
     */
    inline static void obj_free(void * obj) { ::free(obj); }

    /**
     *  \brief  Free objects in list
     *
     *  \param  obj_list  List of objects
     */
    inline static void objlist_free(void * obj_list) {
        while (NULL != obj_list) {
            void * obj = obj_list;
            obj_list = *((void **)obj);

            obj_free(obj);
        }
    }

    /**
     *  \brief  Initialise object pool
     *
     *  \param  size      Required object size
     *  \param  prealloc  Pre-allocated objects count (default: 0)
     *  \param  plimit    Pooled objects limit (default: unlimited)
     *  \param  tlimit    Total objects limit (default: unlimited)
     */
    void init(
        size_t size,
        size_t prealloc = 0,
        size_t plimit   = SIZE_MAX,
        size_t tlimit   = SIZE_MAX) throw(std::logic_error);

#ifdef HAVE_CXX11
    /** Swap with rvalue (no locking done) */
    void swap(objpool && rval);
#endif  // end of #ifdef HAVE_CXX11

    public:

    /**
     *  \brief  Constructor
     *
     *  Object size has to be specified.
     *
     *  Other optional parameters semantics:
     *  * prealloc  How many objects shall be pre-allocated
     *              during the pool construction
     *  * plimit    Pooled objects limit.  Should the number
     *              of pooled objects exceed the limit (upon object release),
     *              the object responsible shall be freed (returned to system)
     *              rather than pooled (so the limit isn't exceeded).
     *              If lower than \c prealloc, an exception is thrown.
     *  * tlimit    Total objects limit.  Similarly to the \c plimit,
     *              \c tlimit limits total amount of allocated objects.
     *              It may be nicely used e.g. to control amount of
     *              objects that may exist concurrently in a system.
     *
     *  \param  size      Required object size
     *  \param  prealloc  Pre-allocated objects count (default: 0)
     *  \param  plimit    Pooled objects limit (default: unlimited)
     *  \param  tlimit    Total objects limit (default: unlimited)
     */
    objpool(
        size_t size,
        size_t prealloc = 0,
        size_t plimit   = SIZE_MAX,
        size_t tlimit   = SIZE_MAX)
    :
        m_pool(NULL),
        m_size(0),
        m_cnt_pool(0),
        m_cnt_total(0),
        m_plimit(0),
        m_tlimit(0)
    {
        init(size, prealloc, plimit, tlimit);
    }

#ifdef HAVE_CXX11
    /** Move constructor */
    objpool(objpool && orig):
        m_pool(NULL),
        m_size(0),
        m_cnt_pool(0),
        m_cnt_total(0),
        m_plimit(0),
        m_tlimit(0)
    {
        lock4scope(orig.m_mutex);

        swap(std::move(orig));
    }
#endif  // end of #ifdef HAVE_CXX11

    /** Object size getter */
    inline size_t size() const { lock4scope(m_mutex); return m_size; }

    /** Pooled objects count getter */
    inline size_t pooled() const { lock4scope(m_mutex); return m_cnt_pool; }

    /** Total objects count getter */
    inline size_t total() const { lock4scope(m_mutex); return m_cnt_total; }

    /** Used objects count getter */
    inline size_t used() const {
        lock4scope(m_mutex);

        return m_cnt_total - m_cnt_pool;
    }

    /** Pooled objects limit getter */
    inline size_t pool_limit() const { lock4scope(m_mutex); return m_plimit; }

    /** Total objects limit getter */
    inline size_t total_limit() const { lock4scope(m_mutex); return m_tlimit; }

    /**
     *  \brief  Pooled objects limit setter
     *
     *  Sets new pooled objects limit.
     *  Note that the function frees objects that exceed the new limit.
     *  Also note that the new pool limit must not exceed total limit.
     *  If so, an exceptionis thrown.
     *
     *  \param  limit  New limit
     */
    void set_pool_limit(size_t limit) throw(std::logic_error);

    /**
     *  \brief  Total objects limit setter
     *
     *  Sets new total objects limit.
     *  Note that the function frees objects that exceed the new limit.
     *  Also note that the pool limit must not exceed the new total limit.
     *  Also also note that if too many objects are currently in use
     *  (and therefore out of the pool control), the function can't perform
     *  the requested change.
     *  In these cases, an exception shall be thrown.
     *
     *  \param  limit  New limit
     */
    void set_total_limit(size_t limit) throw(std::logic_error);

    /**
     *  \brief  Allocate object from pool
     *
     *  Note that the function throws exception on memory error.
     *
     *  \return Object or \c NULL if limit on total amount of objects was reached
     */
    void * lim_alloc();

    private:

    /**
     *  \brief  Implement allocation (with limit)
     *
     *  IMPORTANT NOTE:
     *  The function assumes that the operation mutex is locked.
     *
     *  \return Memory object or \c NULL if limit was reached
     */
    inline void * lim_alloc_impl();

    public:

    /**
     *  \brief  Allocate object from pool
     *
     *  This version of the allocation allows for locking timeout specification.
     *  Unless the lock is acquired within the specified timeout,
     *  Note that the function throws exception on memory error.
     *
     *  \param  timeout  Lock timeout
     *  \param  busy     Set accordingy to timed lock acquisition success
     *
     *  \return Object or \c NULL if limit on total amount of objects was reached
     */
    void * lim_alloc(double timeout, bool & busy);

    /**
     *  \brief  Allocate object from pool
     *
     *  This function works the same as the previous one,
     *  except it provides the caller with information on the time
     *  required for the pool lock acquisition.
     *
     *  \param  timeout    Lock timeout
     *  \param  lock_time  Lock acquisition time
     *  \param  busy       Set accordingy to timed lock acquisition success
     *
     *  \return Object or \c NULL if limit on total amount of objects was reached
     */
    void * lim_alloc(double timeout, double & lock_time, bool & busy);

    /**
     *  \brief  Allocate object from pool
     *
     *  Note that the function throws exception on memory error
     *  or on reaching of the limit of total amount of objects allocated.
     *
     *  \return Object
     */
    inline void * alloc() throw(std::bad_alloc) {
        void * obj = lim_alloc();

        if (NULL == obj)
            throw std::bad_alloc();

        return obj;
    }

    /**
     *  \brief  Release object back to pool
     *
     *  \param  obj  Object
     */
    void free(void * obj);

    /**
     *  \brief  Clean the pool up
     *
     *  The function frees all objects currently pooled.
     *  Of course, it can't (and won't) do anything about
     *  the objects that are in use at the time.
     *
     *  Note the \c finish flag; if set, the function will set
     *  total limit to 0; that will efectively disable further
     *  allocations even for unlocked pool.
     *  Thus, the pool shall be prepared for destruction.
     *  Correctness of usage (i.e. all objects returned to pool)
     *  is done in this case; exception is thrown on illegal use.
     *
     *  \param  finish  Use \c true for cleanup before destruction
     */
    inline void cleanup(bool finish = false) throw(std::logic_error) {
        void * trash;

        {
            lock4scope(m_mutex);

            trash  = m_pool;
            m_pool = NULL;

            m_cnt_total -= m_cnt_pool;
            m_cnt_pool   = 0;

            if (finish) {
                // Some objects still in use
                if (m_cnt_total)
                    throw std::logic_error("objpool: incorrect usage");

                m_tlimit = 0;
            }
        }

        // Unlocked
        objlist_free(trash);
    }

    /**
     *  \brief  Destructor
     *
     *  Cleans the pool up.
     */
    ~objpool() { cleanup(true); }

#ifdef HAVE_CXX11
    /** Rvalue assignment */
    objpool & operator = (objpool && rval) {
        cleanup(true);

        lock4scope(m_mutex);
        lock4scope(rval.m_mutex);

        swap(std::move(rval));

        return rval;
    }
#endif  // end of #ifdef HAVE_CXX11

    private:

    /** Copying is forbidden */
    objpool(const objpool & orig): m_size(0) {}

    /** Assignment is forbidden */
    void operator = (const objpool & rval) {}

};  // end of class objpool


namespace impl {
    /** Empty functor */
    template <typename T>
    class nop {
        public:

        inline void operator () (T & ) {}
    };
}  // end of namespace impl

/**
 *  \brief  Auto-scaled memory object pool
 *
 *  The class provides paralelisation of the dynamic object pool.
 *
 *  Allocation locks average acquisition time is watched (using sliding
 *  window average).
 *  If the average exceeds defined threshold, another pool is allocated
 *  to widen the parallelisation bottleneck (there is high concurrency
 *  upon the pool).
 *
 *  The pools are kept in a list sorted in ascending order by the avg access
 *  times (a limit on the list length may be imposed).
 *  Acquire operation is always done on the list head.
 *  (Note that the operation that provoked creation of a new pool places it
 *  to the list head and the subsequent re-acquire operation is done on that
 *  very pool.)
 *
 *  Pools may disappear from the list; if a pool gets unused while at the tail
 *  of the list (i.e. it's quite improbable it will be used soon), it shall be
 *  destroyed (in scope of the thread that released the last used object).
 *
 *  When request concurrency is detected to be low (i.e. count of lock times
 *  being under the threshold during the last \c N allocations underflows
 *  a low watermark) then further unused pool is removed upon (last)
 *  object is returned into it.
 */
class ascale_objpool {
    private:

    // Configuration
    const size_t m_size;       /**< Object size                           */
    const size_t m_plimit;     /**< Pooled objects limit (per pool)       */
    const size_t m_tlimit;     /**< Total objects limit (global)          */
    const size_t m_alt_wsize;  /**< Acquire lock time average window size */
    const double m_alt_th;     /**< Acquire lock time threshold           */

    // Implementation
    typedef stats::avg_fwin<double> alt_avg_t;  /**< Acquire lock time avg. */

    /** Pool list item */
    struct objpool_entry {
        size_t           size;       /**< Memory object size                 */
        objpool          pool_impl;  /**< Object pool                        */
        bool             in_use;     /**< Operation currently in progress    */
        alt_avg_t        alt_avg;    /**< The pool acquire lock time average */

        objpool_entry * prev;  /**< Previous entry in list */
        objpool_entry * next;  /**< Next     entry in list */

        /**
         *  \brief  Constructor
         *
         *  \param  mem_size   Memory object size
         *  \param  prealloc   Number of pre-allocated mem. objects in the pool
         *  \param  plimit     Limit on mem. objects in the pool
         *  \param  alt_wsize  Acquire lock time average window size
         */
        objpool_entry(
            size_t mem_size,
            size_t prealloc,
            size_t plimit,
            size_t alt_wsize)
        :
            size(mem_size),
            pool_impl(size, prealloc, plimit, 0),
            in_use(false),
            alt_avg(alt_wsize),
            prev(NULL),
            next(NULL)
        {}

#ifdef HAVE_CXX11
        /**
         *  \brief  Move constructor
         *
         *  IMPORTANT NOTE:
         *  It is only correct to move entries belonging to the same
         *  auto-scaled object pool.
         *  (Otherwise there's no way to set the \c pool_meta reference
         *  correctly.)
         */
        objpool_entry(objpool_entry && orig):
            size(orig.size),
            pool_impl(std::move(orig.pool_impl)),
            in_use(orig.in_use),
            alt_avg(orig.alt_avg),
            prev(orig.prev),
            next(orig.next)
        {}
#endif  // end of #ifdef HAVE_CXX11

    };  // end of struct objpool_entry

    /** Object pool entry list */
    class objpool_entries {
        private:

        objpool         m_pool;  /**< Entry pool */
        objpool_entry * m_head;  /**< List head  */

        public:

        /**
         *  \brief  Constructor (empty list)
         *
         *  \param  pprealloc  Pre-allocated mem. obj. pool entries
         *  \param  pplimit    Pooled mem. obj. pool entries limit
         *  \param  ptlimit    Total mem. obj. pool entries limit
         */
        objpool_entries(
            size_t pprealloc,
            size_t pplimit,
            size_t ptlimit)
        :
            m_pool(sizeof(objpool_entry), pprealloc, pplimit, ptlimit),
            m_head(NULL)
        {}

#ifdef HAVE_CXX11
        /** Move constructor */
        objpool_entries(objpool_entries && rval):
            m_pool(std::move(rval.m_pool)),
            m_head(rval.m_head)
        {
            rval.m_head = NULL;
        }
#endif  // end of #ifdef HAVE_CXX11

        /** Head getter */
        inline objpool_entry * head() const { return(m_head); }

        /**
         *  \brief  Construct new entry
         *
         *  \param  mem_size   Memory object size
         *  \param  prealloc   Number of pre-allocated mem. objects in the pool
         *  \param  plimit     Limit on mem. objects in the pool
         *  \param  alt_wsize  Acquire lock time average window size
         */
        inline objpool_entry * create(
            size_t mem_size,
            size_t prealloc,
            size_t plimit,
            size_t alt_wsize)
        {
            objpool_entry * entry = (objpool_entry *)m_pool.lim_alloc();

            if (NULL != entry) {
                new(entry) objpool_entry(
                    mem_size,
                    prealloc,
                    plimit,
                    alt_wsize);
            }

            return entry;
        }

        /**
         *  \brief  Add entry to list
         *
         *  \param  entry  Entry
         */
        inline void add(objpool_entry * entry) {
            assert(NULL != entry);

            entry->next = m_head;
            m_head      = entry;
        }

        /**
         *  \brief  Remove entry from list
         *
         *  \param  entry  Entry
         */
        void remove(objpool_entry * entry);

        /**
         *  \brief  Destroy entry
         *
         *  \param  entry  Condemned entry
         */
        inline void destroy(objpool_entry * entry) {
            entry->~objpool_entry();

            m_pool.free(entry);
        }

        /**
         *  \brief  Move entry to a diferrent position in list
         *
         *  Note that the position must differ from the current one.
         *
         *  \param  entry  Pool entry
         *  \param  pos    Pool entry position
         */
        void move_to(objpool_entry * entry, objpool_entry * pos);

        /**
         *  \brief  Destructor
         *
         *  Cleaning up is the pool's responsibility.
         *  The list destructor only checks that it was emptied.
         */
        ~objpool_entries() {
            if (NULL != m_head)
                throw std::logic_error(
                    "ascale_objpool: pool entries mess");
        }

    };  // end of class objpool_entries

    /** List of object pools */
    typedef objpool_entries objpools_t;

    /** Dynamic memory object header */
    struct obj_header {
        objpool_entry * pool_entry;  /**< The object pool of origin */
    };  // end of struct obj_header

    objpools_t m_objpools;  /**< Object pool list           */
    size_t     m_objcnt;    /**< Count of allocated objects */

    mutable mt::mutex m_mutex;  /**< Global operations mutex    */

    /**
     *  \brief  Payload address getter
     *
     *  \param  header  Memory object header address
     *
     *  \return Payload address
     */
    inline static void * pl_addr(obj_header * header) {
        return (char *)header + sizeof(obj_header);
    }

    /**
     *  \brief  Memory object header address getter
     *
     *  \param  pl  Payload address
     *
     *  \return Header address
     */
    inline static obj_header * hdr_addr(void * pl) {
        return reinterpret_cast<obj_header *>((char *)pl - sizeof(obj_header));
    }

    /**
     *  \brief  Provide memory object
     *
     *  The function returns address of payload within the memory object.
     *  It also sets the object header.
     *
     *  \param  obj         Memory object (plain)
     *  \param  pool_entry  Entry of the pool of origin
     *
     *  \return Memory object (payload address)
     */
    inline static void * obj_provide(
        void *          obj,
        objpool_entry * pool_entry)
    {
        obj_header * header = reinterpret_cast<obj_header *>(obj);

        header->pool_entry = pool_entry;

        return pl_addr(header);
    }

    /**
     *  \brief  Add new pool
     *
     *  The function adds another pool to list.
     *
     *  NOTE: The function doesn't lock/unlock the global mutex.
     *
     *  \param  prealloc  Number of pre-allocated mem. objects in the pool
     *  \param  in_use    In-use flag for the new pool
     *
     *  \return The new pool entry or \c NULL if limit was reached
     */
    objpool_entry * add_pool(size_t prealloc, bool in_use = false);

    /**
     *  \brief  Remove pool
     *
     *  The function removes a pool.
     *
     *  NOTE: The function doesn't lock/unlock the global mutex.
     *
     *  \param  pool_entry  Pool entry
     */
    void rm_pool(objpool_entry * pool_entry);

    public:

    /** Default average lock time sliding window (history) size */
    static const size_t default_alt_wsize;

    /**
     *  \brief  Constructor
     *
     *  The \c size parameter specifies the allocated mem. object size.
     *
     *  The \c alt_th threashold should be set to a value considered
     *  a high watermark of the allocation lock acquisition time.
     *  If time spent by waiting for the lock exceeds the threshold,
     *  another mem. obj. pool shall be allocated.
     *  Note that the value is NOT a limit; the implementation can't
     *  guarrantee lock acquisition time.
     *  Also note that the time does NOT include the time spent on
     *  the allocation itself; if there's no object cached, system
     *  allocator call is required (which might be quite time-costly).
     *  A cached object, on the other hand, shall be provided very fast
     *  (in O(1) time).
     *
     *  \c alt_wsize specifies length of history of lock acquisition
     *  times used for the average computation.
     *  It therefore affects the "inertia" of the organisation of pools;
     *  higher values mean less changing list of mem. obj. pools.
     *
     *  The \c prealloc, \c plimit and \c tlimit parameters have the same
     *  meaning as with the \ref objpool constructor.
     *
     *  The last 3 parameters are passed to the pool of the mem. obj. pool
     *  list entries.
     *  To avoid startup latency with allocation of the mem. obj. pools
     *  themselves, items of the pool list itself are pre-allocated and cached.
     *  Note that the list should not be too long; also, it's probably not
     *  necessary to pre-allocate all the pool list entries (although for very
     *  short lists, it may be wise).
     *
     *  \param  size       Required object size
     *  \param  alt_th     Allocation lock time threshold (in seconds)
     *  \param  alt_wsize  Allocation lock average time window size
     *  \param  prealloc   Pre-allocated objects count (default: 0)
     *  \param  plimit     Pooled objects limit (default: unlimited)
     *  \param  tlimit     Total objects limit (default: unlimited)
     *  \param  pprealloc  Pre-allocated mem. obj. pool entries
     *  \param  pplimit    Pooled mem. obj. pool entries limit
     *  \param  ptlimit    Total mem. obj. pool entries limit
     */
    ascale_objpool(
        size_t size,
        double alt_th,
        size_t alt_wsize = default_alt_wsize,
        size_t prealloc  = 0,
        size_t plimit    = SIZE_MAX,
        size_t tlimit    = SIZE_MAX,
        size_t pprealloc = 8,
        size_t pplimit   = 16,
        size_t ptlimit   = 16)
    :
        m_size(size),
        m_plimit(plimit),
        m_tlimit(tlimit),
        m_alt_wsize(alt_wsize),
        m_alt_th(alt_th),
        m_objpools(pprealloc, pplimit, ptlimit),
        m_objcnt(prealloc)
    {
        while (0 < prealloc) {
            size_t alloc = prealloc > plimit ? plimit : prealloc;

            add_pool(alloc);

            prealloc -= alloc;
        }
    }

#ifdef HAVE_CXX11
    /** Move constructor */
    ascale_objpool(ascale_objpool && orig):
        m_size(orig.m_size),
        m_plimit(orig.m_plimit),
        m_tlimit(orig.m_tlimit),
        m_alt_wsize(orig.m_alt_wsize),
        m_alt_th(orig.m_alt_th),
        m_objpools(std::move(orig.m_objpools)),
        m_objcnt(orig.m_objcnt)
    {
        orig.m_objcnt = 0;
    }
#endif  // end of #ifdef HAVE_CXX11

    /** Object size getter */
    inline size_t size() const { return m_size; }

    /** Allocated objects count getter */
    inline size_t allocated() const {
        lock4scope(m_mutex);

        return m_objcnt;
    }

    /** Pooled objects limit getter */
    inline size_t pool_limit() const { return m_plimit; }

    /** Total objects limit getter */
    inline size_t total_limit() const { return m_tlimit; }

    /**
     *  \brief  Alocate object from pool
     *
     *  If the pool on the pool list head is so loaded that access lock
     *  is unavailable within the acquire lock time threshold, a new pool
     *  is created and the object is requested from the new one.
     *
     *  Note that the function throws exception on memory error.
     *
     *  \return Object or \c NULL if limit on total amount of objects was reached
     */
    void * lim_alloc();

    /**
     *  \brief  Allocate object from pool
     *
     *  Note that the function throws exception on memory error
     *  or on reaching of the limit of total amount of objects allocated.
     *
     *  \return Memory object (payload address)
     */
    inline void * alloc() throw(std::bad_alloc) {
        void * obj = lim_alloc();

        if (NULL == obj)
            throw std::bad_alloc();

        return obj;
    }

    /**
     *  \brief  Release object back to pool
     *
     *  \param  obj  Memory object (payload address)
     */
    inline void free(void * obj) {
        obj_header * header = hdr_addr(obj);

        header->pool_entry->pool_impl.free(header);

        lock4scope(m_mutex);

        --m_objcnt;  // now we're sure the object was freed...
    }

    /**
     *  \brief  Get memory object size
     *
     *  \param  obj  Memory object (payload address)
     */
    inline static size_t size(void * obj) {
        return hdr_addr(obj)->pool_entry->size - sizeof(obj_header);
    }

    private:

    /** Assignment is prohibited */
    void operator = (const ascale_objpool & orig) {}

};  // end of class ascale_objpool


/**
 *  \brief  Dynamic memory pool
 *
 *  The pool is built upon a vector of auto-scaled object-pools, each providing
 *  objects of different size.
 *
 *  The \c selector template parameter is used to provide the number of object
 *  pools (i.e. count of different sizes), the sizes specifications per each
 *  vector index and translation of every specified size to appropriate index.
 *  Typically, index of closest greater or equal size object pool shall be
 *  assigned.
 *  Note that when the object is freed, the actual size claimed by the object
 *  shall be used to find the pool index; it is therefore essential that
 *  if the selector provides index \c i for requested size \c s and the pool
 *  at \c i provides objects of size \c S (>= s) then the selector MUST also
 *  provide index \c i for requested size \c S.
 *
 *  Typical implementation of selector might be simply to divide the size scale
 *  to \c n segments and provide the closest greater or equal size index.
 *  However, it might be interesting to use other than constant function
 *  to define the segment sizes (i.e. the segments don't have to represent
 *  the same amount of sizes).
 *  They might progressively incerase (geometric or exponential sequencing),
 *  they may be distributed normally around a statistically prominent size
 *  (Gaussian function) etc...
 *
 *  If requested size is too big, the pool falls back to using the system
 *  allocator.
 *
 *  The \c selector interface follows:
 *
 *   class selector {
 *       public:
 *       size_t max_size();
 *       size_t size_cnt();
 *       size_t size(size_t index);
 *       size_t index(size_t size);
 *   };
 *
 *  Formal requirements on \c selector implementation:
 *  * selector is copied by the pool constructor
 *  * sel.size_cnt() > max { sel.index(s) | s <= sel.max_size() }
 *  * s <= sel.size(sel.index(s))
 *  * sel.index(s) == i && sel.size(i) == S => sel.index(S) == i
 *
 *  Obviously, the last 2 are required for s,S <= sel.max_size()
 */
template <class selector>
class mempool {
    private:

    /** Object pool vector type */
    typedef std::vector<ascale_objpool> objpools_t;

    objpools_t m_objpools;  /**< Vector of object pools (different sizes) */
    selector   m_selector;  /**< Pool selector & size distribution        */

    public:

    /** Default lock acquisition time average window size */
    static const size_t default_alt_wsize;

    /**
     *  \brief  Constructor
     *
     *  \param  sel        Selector
     *  \param  alt_th     Lock acquisition time threshold
     *  \param  alt_wsize  Lock acquisition time average window size
     *  \param  climit     Concurrency limit
     */
    mempool(
        const selector & sel,
        double           alt_th,
        size_t           alt_wsize = default_alt_wsize,
        size_t           climit    = 8)
    :
        m_selector(sel)
    {
        size_t size_cnt = m_selector.size_cnt();

        m_objpools.reserve(size_cnt);

        for (size_t i = 0; i < size_cnt; ++i) {
            size_t size = m_selector.size(i);

            m_objpools.push_back(
                ascale_objpool(
                    size, alt_th, alt_wsize,

                    // Pools: no preallocation, no limits
                    0, SIZE_MAX, SIZE_MAX,

                    // Pool entries: no preallocation
                    0, climit, climit));
        }
    }

    /**
     *  \brief  Allocate dynamic memory
     *
     *  \param  size  Required memory size
     *
     *  \return Dynamic memory chunk
     */
    inline void * alloc(size_t size) {
        size_t i = m_selector.index(size);

        return m_objpools.at(i).alloc();
    }

    /**
     *  \brief  Free dynamic memory
     *
     *  \param  mem  Dynamic memory chunk
     */
    inline void free(void * mem) {
        size_t i = m_selector.index(ascale_objpool::size(mem));

        m_objpools.at(i).free(mem);
    }

};  // end of class mempool


#if (0)  // TODO: what about this?  mempool needs selector
/**
 *  \brief  mempool-based allocator
 */
template <typename T>
class mempool_allocator {
    public:

    typedef T                  value_type;
    typedef value_type *       pointer;
    typedef const value_type * const_pointer;
    typedef value_type &       reference;
    typedef const value_type & const_reference;
    typedef std::size_t        size_type;
    typedef std::ptrdiff_t     difference_type;

    template <typename O>
    struct rebind { typedef mempool_allocator<O> other; };

    private:

    mempool & m_pool;  /**< Memory pool (allocation source) */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  pool  Memory pool
     */
    mempool_allocator(mempool & pool): m_pool(pool) {}

    /** Copy constructor */
    mempool_allocator(const mempool_allocator & orig):
        m_pool(orig.m_pool) {}

    /** Address of the object */
    inline pointer address(reference x) const { return &x; }

    /** Constant address of the object */
    inline const_pointer address(const_reference x) const {
        return &x;
    }

    /** Max. amount of objects allocated in array */
    inline size_type max_size() const { return 1; }

    /** Allocate object */
    inline pointer allocate(size_type n, const_pointer hint = 0) {
        assert(1 == n);  // we never allocate more than 1 object at once

        return reinterpret_cast<pointer>(m_pool.alloc());
    }

    /** Deallocate object */
    inline void deallocate(pointer p, size_type n) {
        assert(1 == n);  // we've never allocated more than 1 object at once

        m_pool.free(p);
    }

    /** Object constructor (in-place) */
    inline void construct(
        pointer p,
        size_t  size,
        size_t  prealloc,
        size_t  plimit,
        size_t  alt_wsize)
    {
        new(p) value_type(size, prealloc, plimit, alt_wsize);
    }

    /** Object destructor */
    inline void destroy(pointer p) { p->~value_type(); }

    private:

    /** Object constructor using object copying is forbidden */
    void construct(pointer p, const value_type & value) {}

};  // end of class mempool_allocator
#endif  // end of #if (0)

}  // end of namespace dynamic

#endif  // end of #ifndef dynamic__mempool_hxx
