#ifndef dynamic__pointer_hxx
#define dynamic__pointer_hxx

/**
 *  \brief  Smart pointers
 *
 *  The module contains template definitions of "smart" pointers,
 *  i.e. pointers that keep track of certain properties.
 *
 *  Two main smart pointers are broadly used:
 *  * Unique pointer (formerly autopointer)
 *  * Shared pointer
 *
 *  Both these smart poniters keep track of the dynamic object it pointes to
 *  and ensure both the pointer validity and that the object is destroyed
 *  when not pointed to any longer.
 *
 *  Unique pointer makes sure that as long as the dynamic object lives,
 *  there's exactly one pointer pointing at it.
 *  When the valid pointer is destroyed, the object is destroyed, too.
 *  Unique pointer is beneficial namely in situations when a dynamic
 *  object has to be valid within a certain scope and then automatically
 *  destroyed (no matter how and where the scope is left).
 *
 *  Shared pointer allows more (shared) pointers to point at an object,
 *  counting all such references.
 *  When last of the pointers is destroyed, the object is destroyed, too.
 *  Shared pointer is practically irreplaceable in situations when an object
 *  destruction point can't be deterministically set (predicted).
 *  The object stays valid as long as any entity holds the shared pointer
 *  that references it (and no longer).
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/10/26
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
