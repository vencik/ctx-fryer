#ifndef sys__time_hxx
#define sys__time_hxx

/**
 *  \brief  Time & clock
 *
 *  Time-related operations.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/06/18
 *
 *  Legal notices
 *
 *  Copyright 2014 Vaclav Krpec
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


#include "ctime"


namespace sys {

/** Ticking clock */
class timer {
    private:

    struct timespec m_stamp;  /**< Time stamp */

    inline void gettime(struct timespec & now) throw(std::runtime_error) {
        if (clock_gettime(CLOCK_MONOTONIC, &now))
            throw std::runtime_error(
                "sys::timer: failed to get monotonic time");
    }

    public:

    /** Start timer */
    inline void start() { gettime(m_stamp); }

    /** Get time */
    inline double elapsed() {
        struct timespec now; gettime(now);

        double sec = now.tv_sec - m_stamp.tv_sec;
        sec += (double)(now.tv_nsec - m_stamp.tv_nsec) / 1000000000.0;

        return sec;
    }

};  // end of class timer

}  // end of namespace sys

#endif  // end of #ifndef sys__time_hxx
