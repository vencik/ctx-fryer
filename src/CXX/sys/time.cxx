/**
 *  \brief  Time & clock
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2015/02/06
 *
 *  Legal notices
 *
 *  Copyright 2015 Vaclav Krpec
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


#include "sys/time.hxx"

#include <ctime>


namespace sys {

// timer members

timer::time timer::current_time(timezone_t timezone)
    throw(std::logic_error)
{
    timer t(realtime);

    struct timespec now_ts;
    t.gettime(now_ts);

    struct tm now_up;  // unpacked
    if (UTC == timezone)
        ::gmtime_r(&now_ts.tv_sec, &now_up);
    else if (localtime == timezone)
        ::localtime_r(&now_ts.tv_sec, &now_up);
    else
        throw std::logic_error(
            "sys::timer: unsupported timezone");

    return time(
        (unsigned)now_up.tm_year + 1900,
        (unsigned)now_up.tm_mon + 1,
        (unsigned)now_up.tm_mday,
        (unsigned)now_up.tm_hour,
        (unsigned)now_up.tm_min,
        (unsigned)now_up.tm_sec,
        (unsigned)now_ts.tv_nsec);
}

}  // end of namespace sys
