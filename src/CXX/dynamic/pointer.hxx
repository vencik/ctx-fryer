#ifndef dynamic__pointer_hxx
#define dynamic__pointer_hxx


#include "mt/atomic.hxx"


namespace dynamic {

/**
 *  \brief  Shared pointer
 *
 *  Pointer to shared dynamic object with reference couter.
 *  Copy constructor, assignment etc. increments the counter.
 *  Destructor decrements it.
 *  The underlaying object is destroyed if and only if the reference
 *  count drops to 0.
 *
 *  The reference counting implementation is thread-safe.
 *  Note however, that access to the shared object itself isn't
 *  (unless made so).
 *
 *  For more info, see http://en.wikipedia.org/wiki/Smart_pointer
 */
template <typename T>
class shared_ptr {
    private:

    /** Shared data carrier */
    struct impl {
        T *            obj_ptr;  /**< Shared object pointer */
        mt::atomic_int ref_cnt;  /**< Reference counter     */

        /** Constructor */
        impl(T * ptr): obj_ptr(ptr), ref_cnt(1) {}

    };  // end of struct impl

    impl * m_impl;  /**< Implementation */

    /** Increment reference counter */
    inline void inc_ref_cnt() {
        m_impl->ref_cnt++;  // post-inc is easier then pre-inc
    }

    /**
     *  \brief  Decrement reference counter
     *
     *  Destroys shared data if the ref. counter falls to 0.
     */
    inline void dec_ref_cnt() {
        if (1 == m_impl->ref_cnt--) {
            delete m_impl->obj_ptr;
            delete m_impl;
        }
    }

    public:

    /** Constructor of 1st instance */
    shared_ptr(T * ptr): m_impl(new impl(ptr)) {}

    /** Copy constructor */
    shared_ptr(const shared_ptr & orig):
        m_impl(orig.m_impl)
    {
        inc_ref_cnt();
    }

    /** Assignment */
    shared_ptr & operator = (const shared_ptr & rval) {
        dec_ref_cnt();

        m_impl = rval.m_impl;

        inc_ref_cnt();
    }

    /** Destructor */
    ~shared_ptr() { dec_ref_cnt(); }

    /**
     *  \brief  Indirection
     *
     *  IMPORTANT NOTE:
     *  Indirection allows direct access to the shared object.
     *  That also means that NORMAL pointer (or reference) may be
     *  obtained via indirection.
     *  Use with caution; if you don't want to allow that at all,
     *  derive your own descendant and re-declare indirection
     *  private.
     *  This implementation is permissive... ;-)
     */
    inline T & operator * () { return *(m_impl->obj_ptr); }

    /** Dereference */
    inline T * operator -> () { return m_impl->obj_ptr; }

    private:

    /** Shared pointer only passes by value */
    void operator & () {}

};  // end of class shared_ptr

}  // end of namespace dynamic

#endif  // end of #ifndef dynamic__pointer_hxx
