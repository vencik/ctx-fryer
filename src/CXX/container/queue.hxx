#ifndef container__queue_hxx
#define container__queue_hxx

/**
 *  \brief  FIFO and priority queue
 *
 *  The module contains template definitions of classic FIFO queue
 *  and priority queue.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/11/13
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
#include "heap.hxx"

#include "meta/postpone.hxx"

#include <list>
#include <stdexcept>


namespace container {

/**
 *  \brief  FIFO queue
 *
 *  Classic First-In-First-Out queue.
 *
 *  TODO: Implement \c iterator and \c const_iterator
 */
template <typename T>
class queue {
    private:

    typedef std::list<T> impl_t;  /**< Implementation type */

    impl_t m_impl;  /**< Implementation */

    public:

    /** Size getter */
    inline size_t size() const { return m_impl.size(); }

    /** Queue empty check */
    inline bool empty() const { return m_impl.empty(); }

    /** Head item getter (no item copying) */
    inline const T & head() const { return m_impl.front(); }

    /**
     *  \brief  Pop head item from queue
     *
     *  The function provides the queue head item and removes it.
     *  Note that the operation requires item copying.
     *
     *  \param  item  Popped item
     *
     *  \retval true  on success
     *  \retval false if the queue is empty
     */
    inline bool pop(T & item) {
        if (empty()) return false;

        auto begin = m_impl.begin();

#ifdef HAVE_CXX11
        item = std::move(*begin);
#else
        item = *begin;
#endif  // end of #ifdef HAVE_CXX11

        m_impl.erase(begin);

        return true;
    }

    /**
     *  \brief  Pop head item from queue
     *
     *  If the queue is empty, an exception is thrown.
     *  Otherwise, the function removes the queue head item.
     */
    inline void pop() throw(std::range_error) {
        if (empty())
            throw std::range_error("pop from an empty queue");

        m_impl.erase(m_impl.begin());
    }

    /**
     *  \brief  Push new item to queue (tail)
     *
     *  \param  item  New queue tail
     */
    inline void push(const T & item) { m_impl.push_back(item); }

#ifdef HAVE_CXX11
    /**
     *  \brief  Push new item to queue (tail)
     *
     *  \param  args  Item constructor arguments
     */
    template <typename... Args>
    inline void push(Args... args) { m_impl.emplace_back(args...); }
#endif  // end of #ifdef HAVE_CXX11

};  // end of template class queue


/**
 *  \brief  Priority queue
 *
 *  The priority queue expects that \c Less comparison function is defined
 *  on the template type (with normal behaviour).
 *
 *  The queue \c pop function provides item with lowest value
 *  (with respect to the \c Less function).
 *
 *  TODO: Implement \c iterator and \c const_iterator
 */
template <typename T>
class pqueue {
    private:

    typedef binomial_heap<T> impl_t;  /**< Implementation type */

    impl_t m_impl;  /**< Implementation */

    public:

    /** Size getter */
    inline size_t size() const { return m_impl.size(); }

    /** Queue empty check */
    inline bool empty() const { return m_impl.empty(); }

    /** Head item getter (no item copying) */
    inline const T & head() const { return m_impl.get_min(); }

    /**
     *  \brief  Pop head item from queue
     *
     *  The function provides the queue head item and removes it.
     *  Note that the operation requires item copying.
     *
     *  \retval true  on success
     *  \retval false if the queue is empty
     */
    inline bool pop(T & item) {
        if (empty()) return false;

        item = m_impl.get_min();

        m_impl.delete_min();

        return true;
    }

    /**
     *  \brief  Pop head item from queue
     *
     *  If the queue is empty, an exception is thrown.
     *  Otherwise, the function removes the queue head item.
     */
    inline void pop() throw(std::range_error) {
        if (empty())
            throw std::range_error("pop from an empty queue");

        m_impl.delete_min();
    }

    /**
     *  \brief  Push new item to queue (tail)
     *
     *  \param  item  New queue tail
     */
    inline void push(const T & item) { m_impl.add(item); }

};  // end of template class pqueue

}  // end of namespace container

#endif  // end of #ifndef container__queue_hxx
