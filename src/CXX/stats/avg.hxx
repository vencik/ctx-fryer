#ifndef stats__avg_hxx
#define stats__avg_hxx

/**
 *  \brief  Average computation
 *
 *  The module contains efficient way of computing average.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/02/07
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

#include <vector>
#include <stdexcept>
#include <cassert>


namespace stats {

/**
 *  \brief  Floating window average
 *
 *  The template class allows for computation of average
 *  of last \c n values of a value sequence.
 */
template <typename T>
class avg_fwin {
    private:

    typedef std::vector<T> window_t;  /**< Floating window type */

    window_t m_win;   /**< FLoating window             */
    T        m_sum;   /**< Current values sum          */
    size_t   m_vcnt;  /**< Count of valid window cells */

    window_t::iterator m_pos;  /**< Current window position */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  n  Window size (must be \c >0)
     */
    avg_fwin(size_t n):
        m_sum(0),
        m_vcnt(0),
        m_pos(m_win.end())
    {
        if (0 == n)
            throw std::logic_error("avg_fwin: 0 length window requested");

        m_win.reserve(n);
        m_pos = m_win.begin();
    }

#ifdef HAVE_CXX11
    /** Move constructor */
    avg_fwin(avg_fwin && orig):
        m_win(std::move(orig.m_win)),
        m_sum(orig.m_sum),
        m_vcnt(orig.m_vcnt),
        m_pos(orig.m_pos)
    {
        orig.m_sum  = 0;
        orig.m_vcnt = 0;
        orig.m_pos  = orig.m_win.end();
    }
#endif  // end of #ifdef HAVE_CXX11

    /**
     *  \brief  Push value to window
     *
     *  \param  val  Value
     *
     *  \return Current average
     */
    inline T push(T val) {
        size_t win_size = m_win.size();

        if (!win_size)
            throw std::logic_error("avg_fwin::push: 0 length window");

        if (m_vcnt < win_size)
            ++m_vcnt;
        else
            m_sum -= *m_pos;

        m_sum += *m_pos = val;

        // Shift position
        ++m_pos;

        if (m_pos == m_win.end())
            m_pos = m_win.begin();

        return m_sum / m_vcnt;
    }

    /**
     *  \brief  Average getter
     *
     *  \return Current average or \c 0 on empty window
     */
    inline operator T () const {
        return m_vcnt ? m_sum / m_vcnt : 0;
    }

    /**
     *  \brief  Average getter
     *
     *  \param  valid  Average validity flag
     *
     *  \return Current average or \c 0 on empty window
     */
    inline T get(bool & valid) const {
        return valid = 0 != m_vcnt ? m_sum / m_vcnt : 0;
    }

    /** Valid values count getter */
    inline size_t valid_cnt() const { return m_vcnt; }

    private:

    /** Copying is forbidden */
    avg_fwin(const avg_fwin & orig) {}

    /** Assignment is forbidden */
    void operator = (const avg_fwin & orig) {}

};  // end of template class avg_fwin

}  // end of namespace stats

#endif  // end of #ifndef stats__avg_hxx
