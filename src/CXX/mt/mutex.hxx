#ifndef mt__mutex_hxx
#define mt__mutex_hxx

#include <stdexcept>

extern "C" {
#include <pthread.h>
}


/**
 *  \brief  Lock mutex till end of scope
 *
 *  \param  mutex  Mutex
 */
#define lock4scope(mutex) mt::scopelock __scope_lock_ # __LINE__(mutex)

/**
 *  \brief  Unlock mutex till end of scope
 *
 *  \param  mutex  Mutex
 */
#define unlock4scope(mutex) mt::scopeunlock __scope_unlock_ # __LINE__(mutex)


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
    mutex & operator = (const mutex & orig) {}

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

}  // end of namespace mt

#endif  // end of #ifndef mt__mutex_hxx
