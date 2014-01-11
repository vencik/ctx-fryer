#ifndef mt__atomic_hxx
#define mt__atomic_hxx

/**
 *  \brief  Atomic operations
 *
 *  The module contains definition of types supporting atomic operations.
 *
 *  IMPLEMENTATION NOTES:
 *  The implementation is based on the atomic_ops library by HP.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/12/15
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

extern "C" {
#include <atomic_ops.h>
}


namespace mt {

/**
 *  \brief  Atomic integer
 *
 *  The template class implements an atomic integer.
 */
class atomic_int {
    private:

    volatile AO_t m_impl;  /**< Implementation */

    public:

    typedef AO_t local;  /**< Compatible local value */

    /** Constructor of 0 */
    atomic_int() { AO_store_release_write(&m_impl, 0); }

    /** Constructor */
    atomic_int(local i) { AO_store_release_write(&m_impl, i); }

    /** Conversion to local value */
    inline operator local () const { return AO_load_acquire(&m_impl); }

    /** Copy constructor */
    atomic_int(const atomic_int & orig) {
        local i = orig;

        AO_store_release_write(&m_impl, i);
    }

    /** Local value assignment */
    inline atomic_int & operator = (local i) {
        AO_store_release_write(&m_impl, i);

        return *this;
    }

    /**
     *  \brief  Atomic post-increase
     *
     *  rval = *this; *this += op; return rval;
     *
     *  \param  op
     *
     *  \return Original value
     */
    inline local fetch_and_add(local op) {
        return AO_fetch_and_add_full(&m_impl, op);
    }

    /**
     *  \brief  Atomic increase
     *
     *  *this += op;
     *
     *  \param  op
     *
     *  \return New value (local!)
     */
    inline local operator += (local op) {
        local orig = fetch_and_add(op);

        return orig + op;
    }

    /**
     *  \brief  Atomic decrease
     *
     *  *this -= op;
     *
     *  \param  op
     *
     *  \return New value (local!)
     */
    inline local operator -= (local op) {
        return *this += -op;
    }

    /**
     *  \brief  Atomic pre-incrementation
     *
     *  return ++*this;
     *
     *  \return New value (local!)
     */
    inline local operator ++ () {
        return *this += 1;
    }

    /**
     *  \brief  Atomic post-incrementation
     *
     *  return *this++;
     *
     *  \return Original value (local!)
     */
    inline local operator ++ (int) {
        return fetch_and_add(1);
    }

    /**
     *  \brief  Atomic pre-decrementation
     *
     *  return --*this;
     *
     *  \return New value (local!)
     */
    inline local operator -- () {
        return *this -= 1;
    }

    /**
     *  \brief  Atomic post-decrementation
     *
     *  return *this--;
     *
     *  \return Original value (local!)
     */
    inline local operator -- (int) {
        return fetch_and_add(-1);
    }

    private:

    /**
     *  \brief  Assignment is forbidden
     *
     *  The reason is that underlaying implementation
     *  doesn't support atomic load & store (and no wonder).
     *  Doing sequence of (even atomic) load and store, i.e.:
     *
     *  local i = a; b = i;
     *
     *  would permit the following when thread T1 would do a = b
     *  while thread T2 would do b = a:
     *
     *  T1: local i = a;
     *  T2: local i = b;
     *  T1: b = i;
     *  T2: a = i;
     *
     *  Obviously, that would result in swapping of a and b the values.
     *  But that's wrong; the outcome should be an equal value (of a or b,
     *  depending on which of the operations would be performed first).
     */
    inline void operator = (const atomic_int & orig) {}

};  // end of class atomic_int

}  // end of namespace mt

#endif  // end of #ifndef mt__atomic_hxx
