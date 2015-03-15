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


#include <stdexcept>
#include <ctime>
#include <cstring>


namespace sys {

/** Ticking clock */
class timer {
    public:

    /** Used clock */
    enum clock_t {
        monotonic = CLOCK_MONOTONIC,  /**< Monotonic clock */
        realtime  = CLOCK_REALTIME,   /**< Realtime  clock */
    };

    /** Timezone */
    enum timezone_t {
        localtime = -1,    /** Local time */
        UTC       =  0,    /** UTC (GMT)  */
        GMT       =  UTC,  /** UTC (GMT)  */
    };

    /** Unpacked time */
    class time {
        friend class timer;

        public:

        const unsigned year;    /**< Year                       */
        const unsigned month;   /**< Month (1 - 12)             */
        const unsigned day;     /**< Day of month (1 - 31)      */
        const unsigned hour;    /**< Hour (0 - 23)              */
        const unsigned minute;  /**< Minute (0 - 59)            */
        const unsigned second;  /**< Second (0 - 59)            */
        const unsigned nsec;    /**< Nanosecond (0 - 999999999) */

        private:

        /** Constructor */
        time(
            unsigned _year,
            unsigned _month,
            unsigned _day,
            unsigned _hour,
            unsigned _minute,
            unsigned _second,
            unsigned _nsec)
        :
            year   ( _year   ),
            month  ( _month  ),
            day    ( _day    ),
            hour   ( _hour   ),
            minute ( _minute ),
            second ( _second ),
            nsec   ( _nsec   )
        {}

    };  // struct time

    private:

    const clock_t   m_clock;  /**< Used clock */
    struct timespec m_stamp;  /**< Time stamp */

    inline void gettime(struct timespec & now) throw(std::runtime_error) {
        if (clock_gettime(m_clock, &now))
            throw std::runtime_error(
                "sys::timer: failed to get time");
    }

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  clock  Used clock
     */
    timer(clock_t clock = monotonic): m_clock(clock) {}

    /** Copy constructor */
    timer(const timer & orig): m_clock(orig.m_clock) {
        ::memcpy(&m_stamp, &orig.m_stamp, sizeof(m_stamp));
    }

    /** Start timer */
    inline void start() { gettime(m_stamp); }

    /**
     *  \brief  Set time (for timeouts)
     *
     *  \param  diff  Time difference from now in seconds
     */
    inline void set(double diff) {
        time_t sec  = (time_t)diff;
        long   nsec = 1000000000 * (diff - (double)sec);

        start();

        m_stamp.tv_sec  += sec;
        m_stamp.tv_nsec += nsec;
    }

    /** Get elapsed time */
    inline double elapsed() {
        struct timespec now; gettime(now);

        double sec = now.tv_sec - m_stamp.tv_sec;
        sec += (double)(now.tv_nsec - m_stamp.tv_nsec) / 1000000000.0;

        return sec;
    }

    /** \c struct \c timespec getter (for system operations) */
    inline operator const struct timespec * () { return &m_stamp; }

    /**
     *  \brief  Get current time (unpacked)
     *
     *  \param  timezone  Timezone (GMT/UTC or localtime)
     *
     *  \return Current time
     */
    static time current_time(timezone_t timezone) throw(std::logic_error);

};  // end of class timer

}  // end of namespace sys

#endif  // end of #ifndef sys__time_hxx
