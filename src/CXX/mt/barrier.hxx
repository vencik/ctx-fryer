#ifndef mt__barrier_hxx
#define mt__barrier_hxx

/**
 *  \brief  Barrier
 *
 *  The module contains definition of barrier.
 *
 *  IMPLEMENTATION NOTES:
 *  The implementation is based on POSIX threads library.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/07/24
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
#include <cerrno>

extern "C" {
#include <pthread.h>
}


namespace mt {

/** POSIX thread barrier primitive */
class barrier {
    private:

    pthread_barrier_t m_impl;  /**< POSIX barrier    */
    bool              m_init;  /**< Initialised flag */

    /**
     *  \brief  Initialise POSIX barrier (using default attributes)
     *
     *  \param  cnt  Number of threads synchronised on the barrier
     */
    inline void init_impl(unsigned cnt) throw(std::logic_error) {
        int status = pthread_barrier_init(&m_impl, NULL, cnt);

        if (status)
            throw std::logic_error("POSIX barrier init failed");
    }

    /**
     *  \brief  Destroy POSIX barrier
     */
    inline void destroy_impl() {
        int status = pthread_barrier_destroy(&m_impl);

        switch (status) {
            case 0: return;

            case EBUSY:  // thread(s) waiting on the barrier
                throw std::logic_error("POSIX barrier used on destruction");

            case EINVAL:  // barrier was not initialised
                throw std::logic_error(
                    "POSIX barrier destruction: invalid barrier");

            default:  // ???
                throw std::logic_error(
                    "POSIX barrier destruction: internal error");
        }
    }

    public:

    /**
     *  \brief  Constructor
     *
     *  Note that number of threads synchronised on the barrier isn't set
     *  by the constructor.
     *  This means that the barrier isn't ready for use and requires
     *  calling \ref set_thread_cnt, first.
     *  An attempt to wait on such uninitialised barrier shall result
     *  in an exception thrown.
     */
    barrier(): m_init(false) {}

    /**
     *  \brief  Constructor of barrier with default attributes
     *
     *  \param  cnt  Number of threads synchronised on the barrier
     */
    barrier(unsigned cnt): m_init(true) { init_impl(cnt); }

    /**
     *  \brief  Set the number of threads synchronised on the barrier
     *
     *  The function sets the barrier to ready state for \c cnt threads
     *  synchronisation.
     *
     *  Note that it must never be called if there are any threads
     *  already waiting on the barrier.
     *  Such an attempt shall result in an exception thrown.
     *
     *  Also note that unless you need to change the synced threads count,
     *  you don't need to call the function; the barrier may be directly
     *  reused for the same number of threads.
     *
     *  \param  cnt  Number of threads synchronised on the barrier
     */
    inline void set_thread_cnt(unsigned cnt) {
        if (m_init) destroy_impl();

        init_impl(cnt);
        m_init = true;
    }

    /**
     *  \brief  Wait on barrier
     *
     *  The calling thread is suspended (blocks) until the amount
     *  of threads (including the calling one) that have called
     *  the function reaches the constructor \c cnt parameter.
     *  As soon as that happens, all the threads are resumed.
     *
     *  The function shall return \c true for exactly one of the calling
     *  threads; for the others, \c false shall be returned.
     *  There's no way to predict which one shall be "chosen" this way
     *  in general.
     *  This allows for e.g. selection of a leader, thread that will
     *  perform certain action that needs to be done uniquely (like
     *  re-initialisation of the very barrier) etc.
     *
     *  \return \c true for exactly one waiting thread, \c false for others
     */
    inline bool wait() throw(std::logic_error) {
        int status = pthread_barrier_wait(&m_impl);

        switch (status) {
            case PTHREAD_BARRIER_SERIAL_THREAD: return true;  // the One ;-)
            case 0: return false;  // the ordinary

            default:
                // pthread man pages claim that unless the POSIX barrier
                // is uninitialised (which is done by the constructor),
                // no other error may occur
                throw std::logic_error("POSIX barrier wait: invalid barrier");
        }
    }

    /** Destructor */
    ~barrier() { if (m_init) destroy_impl(); }

    private:

    /** Copying is forbidden */
    barrier(const barrier & orig) {}

    /** Assignment is forbidden */
    void operator = (const barrier & orig) {}

};  // end of class barrier

}  // end of namespace mt

#endif  // end of #ifndef mt__barrier_hxx
