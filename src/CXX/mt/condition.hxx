#ifndef mt__condition_hxx
#define mt__condition_hxx

#include "mt/mutex.hxx"

#include <stdexcept>
#include <cerrno>

extern "C" {
#include <pthread.h>
#include <time.h>
}


namespace mt {

/** POSIX thread condition primitive */
class condition {
    private:

    pthread_cond_t m_impl;  /**< POSIX condition */

    public:

    /** Constructor of condition with default attributes */
    condition() {
        int status = pthread_cond_init(&m_impl, NULL);

        // pthread man pages claim that initialisation never fails
        if (status)
            throw std::logic_error("POSIX condition init failed");
    }

    /**
     *  \brief  Wait on condition
     *
     *  The \c mx attribute is expected to be locked by the calling thread.
     *  The calling thread is suspended (blocks) until the condition
     *  is signalised.
     *  The \c mx is unlocked meanwhile.
     *  When the calling thread is resumed, \c mx is locked again.
     *
     *  \param  mutex  Mutex
     */
    inline void wait(mutex & mx) throw(std::logic_error) {
        int status = pthread_cond_wait(&m_impl, &mx.m_impl);

        // pthread man pages claim that initialisation never fails
        if (status)
            throw std::logic_error("POSIX condition wait failed");
    }

    /**
     *  \brief  Wait on condition (with timeout)
     *
     *  Works like \ref wait but resumes the calling thread anyway
     *  if it was blocked for the timeout specified.
     *
     *  \param  mx       Mutex
     *  \param  timeout  Timeout (in seconds)
     *
     *  \retval \c true  if the condition was signalised
     *  \return \c false if timeout expired or interrupted on signal
     */
    inline bool wait(mutex & mx, double timeout) throw(std::logic_error) {
        struct timespec wake_at;

        int status = clock_gettime(CLOCK_MONOTONIC, &wake_at);

        if (status)
            throw std::logic_error("POSIX condition timeout setting failed");

        time_t sec  = (time_t)timeout;
        long   nsec = 1000000000 * (timeout - (double)sec);

        wake_at.tv_sec  += sec;
        wake_at.tv_nsec += nsec;

        status = pthread_cond_timedwait(&m_impl, &mx.m_impl, &wake_at);

        switch (status) {
            case 0: return true;

            case EINTR:  // interrupted on a signal
            case ETIMEDOUT: return false;

            default:  // ???
                throw std::logic_error("POSIX condition timed wait failed");
        }
    }

    /**
     *  \brief  Signal on condition
     *
     *  Resume (exactly) one of the threads waiting on the condition.
     *  If no threads wait on it, nothing happens (the signal is not
     *  cached in any way).
     *  There's no specification of which thread is woken; simply
     *  one of those waiting...
     */
    inline void signal() throw(std::logic_error) {
        int status = pthread_cond_signal(&m_impl);

        // pthread man pages claim that signalisation never fails
        if (status)
            throw std::logic_error("POSIX condition signal failed");
    }

    /**
     *  \brief  Broadcast signal on condition
     *
     *  Resume all the threads waiting on the condition.
     *  If no threads wait on it, nothing happens (the signal is not
     *  cached in any way).
     */
    inline void broadcast() throw(std::logic_error) {
        int status = pthread_cond_broadcast(&m_impl);

        // pthread man pages claim that broadcasting never fails
        if (status)
            throw std::logic_error("POSIX condition broadcast failed");
    }

    /** Destructor */
    ~condition() {
        int status = pthread_cond_destroy(&m_impl);

        switch (status) {
            case 0: return;

            case EBUSY:  // thread(s) waiting on the condition
                throw std::logic_error("POSIX condition used on destruction");

            default:  // ???
                throw std::logic_error("POSIX condition destruction failed");
        }
    }

    private:

    /** Copying is forbidden */
    condition(const condition & orig) {}

    /** Assignment is forbidden */
    void operator = (const condition & orig) {}

};  // end of class condition

}  // end of namespace mt

#endif  // end of #ifndef mt__condition_hxx
