#ifndef mt__mutex_hxx
#define mt__mutex_hxx

/**
 *  \brief  MutEx
 *
 *  The module contains definition of mutex (mutual exclusion facilitator).
 *
 *  IMPLEMENTATION NOTES:
 *  The implementation is based on POSIX threads library.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/10/18
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

#include "sys/time.hxx"

#include <stdexcept>
#include <cerrno>
#include <cstring>

extern "C" {
#include <pthread.h>
}


/** \cond */
// Instance name constructor for (un)lock4scope and alike
#define _scopelock_name_impl(base, line_no) base ## line_no
#define _scopelock_name(base, line_no) _scopelock_name_impl(base, line_no)
/** \endcond */

/**
 *  \brief  Lock mutex till end of scope
 *
 *  The macro instantiates \ref scopelock, so the argument gets locked
 *  till the scope end.
 *
 *  \param  mutex  Mutex
 */
#define lock4scope(mutex) mt::scopelock _scopelock_name(__scope_lock_, __LINE__)(mutex)

/**
 *  \brief  Unlock mutex till end of scope
 *
 *  Similar to \ref lock4scope, but instantiates \ref scopeunlock
 *  (so that the argument gets unlocked and locked back at scope end).
 *
 *  \param  mutex  Mutex
 */
#define unlock4scope(mutex) mt::scopeunlock _scopelock_name(__scope_unlock_, __LINE__)(mutex)

/**
 *  \brief  Unlock mutex at end of scope
 *
 *  Similar to \ref lock4scope, but instantiates \ref deferredunlock
 *  (so that the argument gets unlocked at scope end).
 *
 *  \param  mutex  Mutex
 */
#define unlockatend(mutex) mt::deferredunlock _scopelock_name(__deferred_unlock_, __LINE__)(mutex)


namespace mt {

/** Mutex */
class mutex {
    friend class condition;

    protected:

    pthread_mutex_t m_impl;  /**< POSIX mutex */

    public:

    /** Constructor of mutex with default attributes */
    mutex() {
        int status = pthread_mutex_init(&m_impl, NULL);

        // Note that pthread man pages claim that init never fails
        if (status)
            throw std::logic_error("POSIX mutex init failed");
    }

    /** Lock mutex */
    inline void lock() throw(std::logic_error) {
        int status = pthread_mutex_lock(&m_impl);

        switch (status) {
            case 0: return;

            case EINVAL:  // not properly initialised
                throw std::logic_error("POSIX mutex not properly initialised");

            case EDEADLK:  // deadlock detection
                throw std::logic_error("POSIX mutex deadlock detected");

            default:  // ???
                throw std::logic_error("POSIX mutex lock failed");
        }
    }

    /**
     *  \brief  Try to lock mutex (without blocking)
     *
     *  \return \c true iff the mutex was locked for caller
     */
    inline bool trylock() throw(std::logic_error) {
        int status = pthread_mutex_trylock(&m_impl);

        switch (status) {
            case 0:     return true;
            case EBUSY: return false;

            case EINVAL:  // not properly initialised
                throw std::logic_error("POSIX mutex not properly initialised");

            default:  // ???
                throw std::logic_error("POSIX mutex trylock failed");
        }
    }

#ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK

/** \ref mt::mutex::trylock with timeout argument is defined */
#define HAVE_MT__MUTEX__TRYLOCK_WITH_TIMEOUT 1

    /**
     *  \brief  Try to lock mutex (with blocking timeout)
     *
     *  The function is similar to \ref trylock, except that it blocks
     *  the calling thread at most \c timeout seconds while trying to
     *  obtain the lock.
     *
     *  \param  timeout  Timeout (in seconds)
     *
     *  \return \c true iff the mutex was locked for caller
     */
    bool trylock(double timeout) throw(std::logic_error) {
        // Note that pthread_mutex_timedlock specifies usage of CLOCK_REALTIME
        sys::timer wake_at(sys::timer::realtime); wake_at.set(timeout);

        int status = pthread_mutex_timedlock(&m_impl, wake_at);

        switch (status) {
            case 0: return true;

            case ETIMEDOUT:  // timeout
            case EAGAIN:     // max. num. of recursive locks exceeded
                return false;

            default:  // ???
                throw std::logic_error("POSIX mutex timed trylock failed");
        }
    }

    /**
     *  \brief  Try to lock mutex (with blocking timeout)
     *
     *  The function works just like \ref trylock with timeout,
     *  except that upon successful acquisition of the lock,
     *  it provides the time the locking took via the last argument.
     *
     *  \param  timeout    Timeout (in seconds)
     *  \param  lock_time  Time required for the lock acquisition
     *
     *  \return \c true iff the mutex was locked for caller
     */
    bool trylock(double timeout, double & lock_time) throw(std::logic_error) {
        // Note that pthread_mutex_timedlock specifies usage of CLOCK_REALTIME
        sys::timer wake_at(sys::timer::realtime); wake_at.set(timeout);

        int status = pthread_mutex_timedlock(&m_impl, wake_at);

        switch (status) {
            case 0:  // compute lock acquisition time
                lock_time = timeout + wake_at.elapsed();

                return true;

            case ETIMEDOUT:  // timeout
            case EAGAIN:     // max. num. of recursive locks exceeded
                return false;

            default:  // ???
                throw std::logic_error("POSIX mutex timed trylock failed");
        }
    }

#endif  // end of #ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK

    /** Unlock mutex */
    inline void unlock() throw(std::logic_error) {
        int status = pthread_mutex_unlock(&m_impl);

        switch (status) {
            case 0: return;

            case EINVAL:  // not properly initialised
                throw std::logic_error("POSIX mutex not properly initialised");

            case EPERM:  // not owned by calling thread
                throw std::logic_error("POSIX mutex invalid unlock");

            default:  // ???
                throw std::logic_error("POSIX mutex unlock failed");
        }
    }

    /** Destructor */
    ~mutex() {
        int status = pthread_mutex_destroy(&m_impl);

        switch (status) {
            case 0: return;

            case EBUSY:  // mutex is locked
                throw std::logic_error("POSIX mutex locked on destruction");

            default:  // ???
                throw std::logic_error("POSIX mutex destruction failed");
        }
    }

    private:

    /** Copying is forbidden */
    mutex(const mutex & orig) {}

    /** Assignment is forbidden */
    void operator = (const mutex & orig) {}

};  // end of class mutex


/**
 *  \brief  Scope lock
 *
 *  Scope lock simplifies mutex locking for flow scope.
 *  Scope lock instance constructor locks a mutex,
 *  its destructor unlocks the mutex.
 *  Since the destructor is always called when leaving
 *  the instance life scope, the caller doesn't have to remember
 *  to unlock the mutex manually.
 *
 *  Best use via the \ref lock4scope macro.
 */
class scopelock {
    private:

    mutex & m_mutex;  /**< Mutex */

    public:

    /** Lock mutex for life of the object */
    scopelock(mutex & mutex): m_mutex(mutex) {
        m_mutex.lock();
    }

    /** Unlock mutex */
    ~scopelock() {
        m_mutex.unlock();
    }

};  // end of class scopelock


/** Reverse of \ref scopelock; also see \ref unlock4scope macro */
class scopeunlock {
    private:

    mutex & m_mutex;  /**< Mutex */

    public:

    /** Unlock mutex for life of the object */
    scopeunlock(mutex & mutex): m_mutex(mutex) {
        m_mutex.unlock();
    }

    ~scopeunlock() {
        m_mutex.lock();
    }

};  // end of class scopeunlock


/**
 *  \brief  Unlock mutex at end of scope
 *
 *  The class instance destructor unlocks the supplied mutex.
 *  That may be conveniently used together with \ref mutex::trylock
 *  operation to automate the mutex unlock on the \c trylock successive
 *  call in cases such as:
 *
 *   if (mx.trylock()) {
 *       deferredunlock du(mx);
 *
 *       // Leaving the scope (in any way) unlocks the mutex
 *   }
 *
 *  The reasoning behind the class is the same as for \ref scopelock.
 *
 *  Best use via the \ref unlockatend macro.
 */
class deferredunlock {
    private:

    mutex & m_mutex;  /**< Mutex */

    public:

    /** Constructor (does nothing to the mutex) */
    deferredunlock(mutex & mutex): m_mutex(mutex) {}

    /** Unlock mutex */
    ~deferredunlock() {
        m_mutex.unlock();
    }

};  // end of class deferredunlock

}  // end of namespace mt

#endif  // end of #ifndef mt__mutex_hxx
