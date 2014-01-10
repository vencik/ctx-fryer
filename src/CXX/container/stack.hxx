#ifndef container__stack_hxx
#define container__stack_hxx

/**
 *  \brief  LIFO stack
 *
 *  The module contains template definition of classic LIFO stack.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/11/16
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

#include <list>
#include <stdexcept>


namespace container {

/**
 *  \brief  LIFO stack
 *
 *  Classic Last-In-First-Out stack.
 *
 *  TODO: Implement \c iterator and \c const_iterator
 */
template <typename T>
class stack {
    private:

    typedef std::list<T> impl_t;  /**< Implementation type */

    impl_t m_impl;  /**< Implementation */

    public:

    /** Size getter */
    inline size_t size() const { return m_impl.size(); }

    /** Stack empty check */
    inline bool empty() const { return m_impl.empty(); }

    /** Top item getter (no item copying) */
    inline const T & top() const { return m_impl.front(); }

    /**
     *  \brief  Pop top item from stack
     *
     *  The function provides the stack top item and removes it.
     *  Note that the operation requires item copying.
     *
     *  \retval true  on success
     *  \retval false if the stack is empty
     */
    inline bool pop(T & item) {
        if (empty()) return false;

        typename impl_t::iterator t = m_impl.begin();

        item = *t;

        m_impl.erase(t);

        return true;
    }

    /**
     *  \brief  Pop top item from stack
     *
     *  If the stack is empty, an exception is thrown.
     *  Otherwise, the function removes the stack top item.
     */
    inline void pop() throw(std::range_error) {
        if (empty())
            throw std::range_error("pop from an empty stack");

        m_impl.erase(m_impl.begin());
    }

    /**
     *  \brief  Push new item to stack (top)
     *
     *  \param  item  New stack top
     */
    inline void push(const T & item) { m_impl.push_front(item); }

};  // end of template class stack

}  // end of namespace container

#endif  // end of #ifndef container__stack_hxx
