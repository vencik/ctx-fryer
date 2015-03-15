#ifndef mt__thread_hxx
#define mt__thread_hxx

/**
 *  \brief  Thread & thread pool
 *
 *  The module contains definition of thread and thread pool (cache).
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/12/23
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

#include "mt/mutex.hxx"
#include "mt/condition.hxx"

#include "container/queue.hxx"
#include "container/stack.hxx"

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <climits>

extern "C" {
#include <unistd.h>
#include <pthread.h>
}


namespace mt {

namespace impl {

/**
 *  \brief  POSIX thread wrapper
 *
 *  The class wraps more or less directly around \c pthread library
 *  thread implementation, including passing of the main routine
 *  as a (static) function pointer.
 */
class POSIX_thread {
    private:

    pthread_t m_impl;  /**< POSIX thread handle */

    public:

    /** Thread attributes */
    class attr {
        friend class POSIX_thread;

        private:

        pthread_attr_t impl;

        public:

        /** Thread detach state */
        enum detach_state_t {
            JOINABLE = PTHREAD_CREATE_JOINABLE,  /**< Thread is joinable */
            DETACHED = PTHREAD_CREATE_DETACHED   /**< Thread is detached */
        };  // end of enum detach_state_t

        /** Thread scope */
        enum scope_t {
            SCOPE_SYSTEM  = PTHREAD_SCOPE_SYSTEM,  /**< System thread  */
            SCOPE_PROCESS = PTHREAD_SCOPE_PROCESS  /**< Process thread */
        };  // end of enum scope_t

        /** Thread scheduling scheme */
        enum sched_scheme_t {
            SCHED_SCHEME_INHERIT  = PTHREAD_INHERIT_SCHED,  /**< Inherited scheduling scheme */
            SCHED_SCHEME_EXPLICIT = PTHREAD_EXPLICIT_SCHED  /**< Explicit  scheduling scheme */
        };  // end of enum sched_scheme_t

        /** Thread scheduling policy */
        enum sched_policy_t {
            SCHED_POLICY_OTHER = SCHED_OTHER,  /**< Other scheduling policy */
            SCHED_POLICY_FIFO  = SCHED_FIFO,   /**< FIFO  scheduling policy */
            SCHED_POLICY_RR    = SCHED_RR      /**< RR    scheduling policy */
        };  // end of enum sched_policy_t

        /** Constructor */
        attr() {
            int status = pthread_attr_init(&impl);

            switch (status) {
                case 0: break;

                case ENOMEM:
                    throw std::runtime_error(
                        "mt::POSIX_thread: Thread attributes allocation failed");

                default:
                    throw std::runtime_error(
                        "mt::POSIX_thread: Thread attributes creation failed");
            }
        }

        /** Destructor */
        ~attr() {
            int status = pthread_attr_destroy(&impl);

            switch (status) {
                case 0: break;

                case ENOMEM:
                    throw std::runtime_error(
                        "mt::POSIX_thread: Thread attributes deallocation failed");

                default:
                    throw std::runtime_error(
                        "mt::POSIX_thread: Thread attributes destruction failed");
            }
        }

        /** Detach state setter */
        void set_detach_state(detach_state_t state) throw(std::runtime_error) {
            int status = pthread_attr_setdetachstate(&impl, (int)state);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to set thread detach state");
        }

        /** Contention scope setter */
        void set_scope(scope_t scope) throw(std::runtime_error) {
            int status = pthread_attr_setscope(&impl, (int)scope);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to set thread scope");
        }

        private:

        /** Scheduling scheme setter */
        void set_sched_scheme(sched_scheme_t scheme) throw(std::runtime_error) {
            int status = pthread_attr_setinheritsched(&impl, (int)scheme);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to set thread scheduling scheme");
        }

        public:

        /** Scheduling policy setter */
        void set_sched_policy(sched_policy_t policy) throw(std::runtime_error) {
            int status = pthread_attr_setschedpolicy(&impl, (int)policy);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to set thread scheduling policy");

            set_sched_scheme(SCHED_SCHEME_EXPLICIT);
        }

        /** Scheduling priority setter */
        void set_sched_priority(int priority) throw(std::runtime_error) {
            struct sched_param sched_params;

            int status = pthread_attr_getschedparam(&impl, &sched_params);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to get thread scheduling parameters");

            sched_params.sched_priority = priority;

            status = pthread_attr_setschedparam(&impl, &sched_params);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to set thread scheduling priority");

            set_sched_scheme(SCHED_SCHEME_EXPLICIT);
        }

        /** Guard size setter */
        void set_guard_size(size_t size) throw(std::runtime_error) {
            int status = pthread_attr_setguardsize(&impl, size);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: failed to set thread guard size");
        }

        /** Stack size setter */
        void set_stack_size(size_t size) throw(std::runtime_error) {
            void * sp;

            int status = posix_memalign(&sp, sysconf(_SC_PAGESIZE), size);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to align thread stack memory");

            status = pthread_attr_setstack(&impl, sp, size);

            if (status)
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to set thread stack");
        }

    };  // end of class attr

    /**
     *  \brief  Start POSIX thread
     *
     *  Throws \c std::runtime_error on memory error or
     *  if thread could not be started for other reasons.
     *
     *  \param  attrs  Thread attributes
     *  \param  main   Thread routine
     *  \param  arg    Thread routine argument (to be passed to \c pthread_create)
     */
    void start(const attr & attrs, void * (* main)(void * ), void * arg)
        throw(std::runtime_error)
    {
        int status = pthread_create(&m_impl, &attrs.impl, main, arg);

        switch (status) {
            case 0: break;

            case EAGAIN:
                throw std::runtime_error(
                    "mt::POSIX_thread: System resources insufficient to create thread");

            case EINVAL:
                throw std::runtime_error(
                    "mt::POSIX_thread: Invalid thread attributes");

            case EPERM:
                throw std::runtime_error(
                    "mt::POSIX_thread: Thread creation denied");

            default:
                throw std::runtime_error(
                    "mt::POSIX_thread: Thread creation failed");
        }
    }

    public:

    /** Constructor of current thread handle */
    POSIX_thread(): m_impl(pthread_self()) {}

    /**
     *  \brief  Constructor (default attributes)
     *
     *  \param  main  Thread routine
     *  \param  arg   Thread routine argument (optional)
     */
    POSIX_thread(void * (* main)(void * ), void * arg = NULL) {
        start(attr(), main, arg);
    }

    /**
     *  \brief  Constructor
     *
     *  \param  attrs  Thread attributes
     *  \param  main   Thread routine
     *  \param  arg    Thread routine argument (optional)
     */
    POSIX_thread(const attr & attrs, void * (* main)(void *), void * arg = NULL) {
        start(attrs, main, arg);
    }

    /** Comparison of threads (negative) */
    inline bool operator != (const POSIX_thread & thread) const {
        return 0 == pthread_equal(this->m_impl, thread.m_impl);
    }

    /** Comparison of threads */
    inline bool operator == (const POSIX_thread & thread) const {
        return !(*this != thread);
    }

    /**
     *  \brief  Join thread
     *
     *  \return Thread routine return value (or \c CANCELED).
     */
    inline void * join() const throw(std::runtime_error) {
        void * retval;

        int status = pthread_join(m_impl, &retval);

        switch (status) {
            case 0: break;

            case EDEADLK:
                throw std::runtime_error(
                    "mt::POSIX_thread: Thread deadlock detected");

            case EINVAL:
                throw std::runtime_error(
                    "mt::POSIX_thread: Can't join thread");

            case ESRCH:
                throw std::runtime_error(
                    "mt::POSIX_thread: Thread not found");

            default:
                throw std::runtime_error(
                    "mt::POSIX_thread: Failed to join thread");
        }
    }

    /** Cancellation type */
    enum cancel_t {
        CANCEL_DEFERRED = PTHREAD_CANCEL_DEFERRED,     /**< Thread may be canceled at cancelation point */
        CANCEL_ASYNC    = PTHREAD_CANCEL_ASYNCHRONOUS  /**< Thread may be canceled at any point         */
    };  // end enum cancel_state_t

    /**
     *  \brief  Cancel thread (if allowed)
     *
     *  If thread cancelation is allowed (see \ref set_cancel), the function
     *  requests thread cancelation (at a cancelation point for deferred cancelation
     *  or immediately for async. cancelation).
     *  See \c man \c 7 \c pthread for POSIX thread cancelation points list.
     *  Note that the cancelation request is asynchronous, i.e. it merely initiates
     *  the cancelation but doesn't wait for its completion.
     *  See \ref cancel for synchronous (blocking) cancelation.
     */
    inline void cancel_async() const throw(std::runtime_error) {
        int status = pthread_cancel(m_impl);

        if (status)
            throw std::runtime_error(
                "mt::POSIX_thread: Failed to cancel thread");
    }

    static const void * CANCELED;  /**< Canceled POSIX thread return value */

    /** Cancel thread and wait until it terminates */
    inline void cancel() const throw(std::runtime_error) {
        cancel();

        if (CANCELED != join())
            throw std::runtime_error(
                "mt::POSIX_thread: Unexpected canceled thread return value");
    }

    /**
     *  \brief  Set cancelability state and type
     *
     *  Applies to calling thread.
     *
     *  \param  enabled  Cancellation enabled / disabled
     *  \param  type     Cancellation type (see \ref cancel_t)
     */
    static void set_cancel(bool enabled, cancel_t type = CANCEL_DEFERRED) {
        int state = enabled ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE;
        int old;

        int status = pthread_setcancelstate(state, &old);

        if (status)
            throw std::runtime_error(
                "mt::POSIX_thread: Failed to set thread cancelation state");

        status = pthread_setcanceltype((int)type, &old);

        if (status)
            throw std::runtime_error(
                "mt::POSIX_thread: Failed to set thread cancelation type");
    }

    /**
     *  \brief  Cancel now if requested to
     *
     *  The function call constitutes an explicit cancelation point.
     *
     *  If the calling thread has been requested to cancel (via \ref cancel
     *  or \ref cancel_async), it cancels now (unless already done so).
     *  May be necessary if the thread doesn't call any function that is
     *  declared to be POSIX cancelation points (see \c man \c 7 \c pthreads)
     *  or if such points are too rare in the flow.
     *  May also be used in conjunction with \ref set_cancel to control
     *  exact flow points where the thread cancelation may be done.
     *
     *  Applies to calling thread.
     */
    inline static void cancel_point() { pthread_testcancel(); }

    /** Detach thread */
    inline void detach() const throw(std::runtime_error) {
        int status = pthread_detach(m_impl);

        if (status)
            throw std::runtime_error(
                "mt::POSIX_thread: Failed to detach thread");
    }

    /**
     *  \brief  Exit calling thread
     *
     *  \param  retval  Thread return value
     */
    inline static void exit(void * retval = NULL) throw() {
        pthread_exit(retval);
    }

};  // end of class POSIX_thread

// POSIX_thread static members initialisers
const void * POSIX_thread::CANCELED = PTHREAD_CANCELED;


/**
 *  \brief  Thread pool (generic implementation)
 *
 *  Thread pool keeps threads that are currently not used but joining
 *  them would be counter-productive since they'll be useful later.
 *  Activating an existing thread stored in the pool is loads faster
 *  than creating a new one.
 *  The threads in the pool are simply suspended waiting on a condition
 *  (new job ready).
 *
 *  The job (a functor) is simply pushed to job queue.
 *  As soon as there's a job in the queue, the condition is signalised
 *  and a pooled thread is woken (or a new one is created if there's
 *  none available).
 *  The thread then processes the job.
 *  When the job ends, the thread returns back to the pool (checking
 *  the job queue first).
 *
 *  The threads may be removed from the pool, either by an explicit call
 *  or automatically if they're pooled for too long.
 *  That's achieved by timed wait on the job condition; if timeout is reached,
 *  the thread leaves the pool and terminates (unless a low limit is reached).
 *  Also, the pool may be used to limit amount of newly created threads;
 *  if its high limit is reached and there's a job in the job queue then
 *  the job waits in gthe queue until a former job is finished.
 *
 *  This template class implements the pooling functionality.
 *  The \c queue_t template parameter is expected to provide implementation of
 *  the \c job queue.
 *  This (common) implementation only expects the following functions being
 *  defined by the queue:
 *
 *   bool empty();
 *   const job & head();
 *   void pop();
 *   void push(const job & );
 */
template <template <class> class queue, class job>
class threadpool {
    public:

    typedef queue<job>         job_queue_t;  /**< Job queue     */
    typedef impl::POSIX_thread worker_t;     /**< Pooled thread */

    private:

    unsigned m_lo;     /**< Minimum of pooled threads        */
    unsigned m_hi;     /**< Maximum of pooled threads        */
    unsigned m_prep;   /**< Actual count of starting threads */
    unsigned m_avail;  /**< Actual count of pooled threads   */
    unsigned m_cnt;    /**< Actual count of managed threads  */
    double   m_ttl;    /**< Pooled thread time to live       */

    bool        m_shutdown;   /**< Shutdown flag             */
    condition   m_wcond;      /**< Worker notification means */
    condition   m_pcond;      /**< Pool notification means   */
    mutex       m_mutex;      /**< Operation mutex           */
    job_queue_t m_job_queue;  /**< Job queue */

    private:

    /** Routine performed by all worker threads (implementation) */
    void worker_routine_impl() {
        lock4scope(m_mutex);

        --m_prep;
        ++m_cnt;

        while (!m_shutdown) {
            // Check job queue
            if (!m_job_queue.empty()) {
                job my_job(m_job_queue.head());

                m_job_queue.pop();

                unlock4scope(m_mutex);

                my_job();  // job processing

                continue;  // check 4 shutdown & another job
            }

            // Wait for another job
            ++m_avail;

            bool signal = m_wcond.wait(m_mutex, m_ttl);

            --m_avail;

            // Put the thread down if too old
            if (!signal && m_lo < m_cnt) break;
        }

        --m_cnt;

        // Last worker shall signalise SD cond. to threadpool
        if (m_shutdown && 0 == m_cnt + m_prep)
            m_pcond.signal();
    }

    /**
     *  \brief  Routine performed by all worker threads
     *
     *  \param  tpool  Thread pool
     *
     *  \return \c NULL
     */
    static void * worker_routine(void * tpool) {
        assert(NULL != tpool);

        threadpool * tp = reinterpret_cast<threadpool *>(tpool);

        try {
            tp->worker_routine_impl();
        }
        catch (std::exception & ex) {
            std::cerr
                << "Threadpool worker routine: std. exception caught: "
                << ex.what()
                << std::endl;

            ::abort();
        }
        catch (...) {
            std::cerr
                << "Threadpool worker routine: unknown exception caught"
                << std::endl;

            ::abort();
        }

        return NULL;
    }

    /**
     *  \brief  Allocate and start another worker thread
     *
     *  The function creates another worker
     *  (unless the pool thread limit would be violated).
     *
     *  \retval false if another worker can't be started
     *  \retval true  on success
     */
    bool start_worker() {
        {
            lock4scope(m_mutex);

            if (m_shutdown) return false;

            unsigned w_cnt = m_cnt + m_prep;

            if (!(w_cnt < m_hi)) return false;

            ++m_prep;  // another worker prepared to start
        }

        try {
            worker_t worker(&worker_routine, this);

            worker.detach();
        }
        catch (std::exception & ex) {
            // TODO: this should be logged

            return false;
        }
        catch (...) {
            std::cerr
                << "Threadpool: caught unexpected exception upon thread start"
                << std::endl;

            ::abort();  // this should never happen
        }

        return true;
    }

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  lo     Threadpool low  pooled threads amount limit
     *  \param  hi     Threadpool high pooled threads amount limit
     *  \param  avail  Preallocated (i.e. pre-started) thread count
     *  \param  ttl    Pooled thread expiration timeout
     */
    threadpool(
        unsigned lo    = 0,
        unsigned hi    = UINT_MAX,
        unsigned avail = 0,
        double   ttl   = 20.0
    ):
        m_lo(lo), m_hi(hi),
        m_prep(0), m_avail(0), m_cnt(0),
        m_ttl(ttl),
        m_shutdown(false)
    {
        // Sanity checks
        if (!(lo <= hi))
            throw std::range_error(
                "mt::threadpool: Limits illegal");

        if (!(lo <= avail && avail <= hi))
            throw std::range_error(
                "mt::threadpool: Preallocation illegal");

        // Preallocate workers
        for (size_t i = 0; i < avail; ++i)
            start_worker();
    }

    /** Shut the pool down (i.e. stop all the threads) */
    void shutdown() {
        lock4scope(m_mutex);

        // Broadcast shutdown
        m_shutdown = true;

        m_wcond.broadcast();

        // Wait for workers to finish
        if (m_cnt + m_prep)
            m_pcond.wait(m_mutex);
    }

    /** Destructor */
    ~threadpool() { shutdown(); }

    /** Job scheduling result */
    enum job_sched_t {
        JOB_SCHED_FAST,       /**< Available thread in pool, job should start quickly */
        JOB_SCHED_NEWTHREAD,  /**< New thread created for the job                     */
        JOB_SCHED_WAIT,       /**< Job has to wait for a thread in queue              */
    };  // end of enum job_sched_t

    /**
     *  \brief  Schedule job
     *
     *  Push another job to job queue.
     *  Jobs are popped from the queue by available threads.
     *  Depending on thread availability and the pool capacity,
     *  the job startup may differ substantially (see return codes).
     *
     *  Note that the return codes should be percieved as hints on
     *  the job scheduling time; in corner cases, \c JOB_SCHED_WAIT
     *  may mean much faster job startup then \c JOB_SCHED_NEWTHREAD
     *  and perhaps even a bit faster then \c JOB_SCHED_FAST.
     *  That will happen when a busy thread becomes available right
     *  after the job was pooled.
     *  Since the threads always check the job queue before going
     *  to sleep in pool, in this case the thread will pop the job
     *  and execute it almost immediately.
     *
     *  \param  j  Job
     *
     *  \retval JOB_SCHED_FAST      there's an available pooled thread,
     *                              job should start pretty fast
     *  \retval JOB_SCHED_NEWTHREAD new thread had to be started for the job
     *  \retval JOB_SCHED_WAIT      no thread is available at the moment,
     *                              the job will have to wait in queue
     */
    job_sched_t run(const job & j) {
        do {  // pragmatic do ... while (0) loop allowing for break
            lock4scope(m_mutex);

            m_job_queue.push(j);

            if (!m_avail) break;

            // A thread is available, pass the job directly to it
            m_wcond.signal();

            return JOB_SCHED_FAST;

        } while (0);  // break target & mutex unlock point

        // Start new thread or wait for an existing one to become available
        return start_worker() ? JOB_SCHED_NEWTHREAD : JOB_SCHED_WAIT;
    }

};  // end of class threadpool


/** LIFO queue */
template <typename T>
class queue_LIFO: public container::stack<T> {
    public:

    /** Head item getter */
    inline const T & head() { return this->top(); }

};  // end of template class queue_LIFO

}  // end of namespace impl


/**
 *  \brief  Thread
 *
 *  The \c Routine template parameter should be a functor with
 *  parentheses operator accepting the thread reference as an argument
 *  and returns \c int as the thread exit code.
 *  The type must define \c Routine::arg_t type for the routine
 *  argument.
 *  The argument is passed by value upon thread construction and stored
 *  in the thread object, where it's available for the routine instance
 *  via a \c thread method.
 */
template <class Routine>
class thread {
    public:

    enum status_t {
        STATUS_FAILED  = -1,  /**< Thread failed to start  */
        STATUS_INIT    =  0,  /**< Thread was initialised  */
        STATUS_STARTUP,       /**< Thread is being started */
        STATUS_RUN,           /**< Thread routine runs     */
        STATUS_DONE,          /**< Thread has finished     */
        STATUS_DETACHED       /**< Thread is detached      */
    };  // end of enum status_t

    private:

    pthread_t               m_impl;    /**< POSIX thread             */
    typename Routine::arg_t m_rarg;    /**< Routine argument         */
    status_t                m_status;  /**< Thread status            */
    int                     m_xcode;   /**< Thread routine exit code */
    mutable mutex           m_mutex;   /**< Operation mutex          */
    condition               m_st_ch;   /**< Status change condition  */

    /**
     *  \brief  Status setter
     *
     *  Sets status and broadcasts the action.
     *
     *  IMPORTANT NOTE:
     *  Must be done under the \c m_mutex locked!
     *
     *  \param  status  Status set
     */
    inline void set_status(status_t status) {
        m_status = status;
        m_st_ch.broadcast();
    }

    public:

    /** Thread id */
    unsigned long id() const { return (unsigned long)m_impl; }

    /** Routine argument getter */
    typename Routine::arg_t & routine_argument() { return m_rarg; }

    /**
     *  \brief  Terminate thread
     *
     *  The function CAN ONLY BE CALLED from the thread routine scope.
     *  It throws an exception if called from another thread.
     *
     *  \param  xcode  Thread exit code
     */
    void exit(int xcode) throw(std::logic_error) {
        pthread_t tid = pthread_self();

        {
            lock4scope(m_mutex);

            // Sanity check
            if (m_impl != tid)
                throw std::logic_error(
                    "mt::thread: Invalid thread exit");

            m_xcode = xcode;

            set_status(STATUS_DONE);
        }

        pthread_exit(NULL);
    }

    private:

    /**
     *  \brief  POSIX thread main routine
     *
     *  \param  tobj  Thread object
     *
     *  \return \c NULL
     */
    static void * main(void * tobj) {
        assert(NULL != tobj);

        int xcode;

        thread * t = (thread *)tobj;

        {
            lock4scope(t->m_mutex);

            t->set_status(STATUS_RUN);
        }

        {   // Routine instance life is limited to this block
            Routine routine;

            xcode = routine(*t);
        }

        t->exit(xcode);  // unless the routine has done it

        return NULL;  // unreachable code
    }

    public:

    /**
     *  \brief  Start thread
     *
     *  The function starts a new POSIX thread.
     *  If the thread was already started, nothing happens
     *  (i.e. the function may be called multiple times;
     *  the thread being only started once, of course).
     *
     *  Note that the function throws an exception in case
     *  previous startup of the tread failed.
     *
     *  \return \c true if and only if the thread was started
     */
    bool start() throw(std::logic_error, std::runtime_error) {
        {
            lock4scope(m_mutex);

            if (STATUS_FAILED == m_status)
                throw std::runtime_error(
                    "mt::thread: Invalid thread start");

            // Already started
            if (STATUS_STARTUP <= m_status) return true;

            assert(STATUS_INIT == m_status);

            set_status(STATUS_STARTUP);  // starting
        }

        pthread_attr_t attr;

        int pt_st = pthread_attr_init(&attr);

        while (0 == pt_st) {  // pragmatic loop allowing for breaks
            pt_st = pthread_create(&m_impl, &attr, &main, this);
            if (0 != pt_st) break;

            pt_st = pthread_attr_destroy(&attr);  // TODO: process status (?)

            return true;
        }

        // Handle memory failure with exception
        if (ENOMEM == pt_st)
            throw std::runtime_error(
                "mt::thread: Failed to init thread attributes");

        // Failed to start thread
        lock4scope(m_mutex);
        set_status(STATUS_FAILED);
        return false;
    }

    /**
     *  \brief  Create thread (default routine argument)
     *
     *  \param  do_start  Whether to start the thread
     */
    thread(bool do_start = true):
        m_impl(0),
        m_status(STATUS_INIT),
        m_xcode(0)
    {
        if (do_start) start();
    }

    /**
     *  \brief  Create thread (passing routine argument)
     *
     *  \param  rarg      Thread routine argument
     *  \param  do_start  Whether to start the thread
     */
    thread(
        const typename Routine::arg_t & rarg,
        bool                            do_start = true)
    :
        m_impl(0),
        m_rarg(rarg),
        m_status(STATUS_INIT),
        m_xcode(0)
    {
        if (do_start) start();
    }

    /** Thread status getter */
    inline status_t status() const {
        lock4scope(m_mutex);

        return m_status;
    }

    /**
     *  \brief  Thread exit code getter
     *
     *  The function throws an exception unless the thread status
     *  is \c STATUS_DONE.
     *  Make sure that the thread has finished before calling the getter.
     */
    inline int xcode() const throw(std::logic_error) {
        lock4scope(m_mutex);

        if (STATUS_DONE != m_status)
            throw std::logic_error(
                "mt::thread: Routine didn't finish");

        return m_xcode;
    }

    /**
     *  \brief  Wait until thread finishes
     *
     *  Note that the function will throw an exception
     *  if the thread is not valid.
     *
     *  \param  status  Status for which to wait (optional)
     *
     *  \return Thread status when done waiting
     */
    status_t wait(status_t status = STATUS_DONE)
        throw(std::logic_error)
    {
        lock4scope(m_mutex);

        if (STATUS_FAILED == m_status)
            throw std::logic_error(
                "mt::thread: Invalid thread wait");

        // Thread doesn't run
        if (STATUS_STARTUP > m_status) return m_status;

        while (status > m_status)
            m_st_ch.wait(m_mutex);

        return m_status;
    }

    /**
     *  \brief  Wait until thread finishes
     *
     *  Note that the function will throw an exception
     *  if the thread is not valid.
     *
     *  \param  timeout  Waiting timeout
     *  \param  status   Status for which to wait (optional)
     *
     *  \return Thread status when done waiting
     */
    status_t wait(double timeout, status_t status = STATUS_DONE)
        throw(std::logic_error)
    {
        lock4scope(m_mutex);

        if (STATUS_FAILED == m_status)
            throw std::logic_error(
                "mt::thread: Invalid thread timeout wait");

        // Thread doesn't run
        if (STATUS_STARTUP > m_status) return m_status;

        // Transitional phase, shouldn't take long
        while (STATUS_STARTUP == m_status)
            m_st_ch.wait(m_mutex);

        if (status > m_status)
            m_st_ch.wait(m_mutex, timeout);

        return m_status;
    }

    /**
     *  \brief  Join thread
     *
     *  Note that the function will throw an exception if the thread
     *  isn't joinable (or the thread failed).
     *
     *  \return \c true if and only if the operation succeeded
     */
    bool join() throw(std::logic_error) {
        {
            lock4scope(m_mutex);

            if (STATUS_FAILED == m_status || STATUS_DETACHED == m_status)
                throw std::logic_error(
                    "mt::thread: Invalid thread join");
        }

        void * retval;
        int pt_st = pthread_join(m_impl, &retval);
        return 0 == pt_st;
    }

    /**
     *  \brief  Detach thread
     *
     *  Note that the function will throw an exception if multiple
     *  detachments are attempted (or the thread failed).
     *
     *  \return \c true if and only if the operation succeeded
     */
    bool detach() throw(std::logic_error) {
        {
            lock4scope(m_mutex);

            if (STATUS_FAILED == m_status || STATUS_DETACHED == m_status)
                throw std::logic_error(
                    "mt::thread: Invalid thread detachment");
        }

        int pt_st = pthread_detach(m_impl);
        if (0 != pt_st) return false;

        lock4scope(m_mutex);
        set_status(STATUS_DETACHED);
        return true;
    }

    /**
     *  \brief  Destructor
     *
     *  Destructor waits for the thread finalisation.
     */
    ~thread() { wait(); }

};  // end of template class thread


/**
 *  \brief  Threadpool with FIFO job queue
 *
 *  FIFO stands for First-In-First-Out; in case the jobs are queued,
 *  they will be executed in order of scheduling times.
 *
 *  See \ref impl::threadpool for common info.
 */
template <typename job>
class threadpool_FIFO: public impl::threadpool<container::queue, job> {};


/**
 *  \brief  Threadpool with priority job queue
 *
 *  In case the jobs are queued, they will be executed in order
 *  of their priority.
 *  The \c job template type must be comparable using \c < operator.
 *  Smaller job means greater priority.
 *
 *  See \ref impl::threadpool for common info.
 */
template <typename job>
class threadpool_priority: public impl::threadpool<container::pqueue, job> {};


/**
 *  \brief  Threadpool with LIFO job queue
 *
 *  LIFO stands for Last-In-First-Out; in case the jobs are queued,
 *  they will be executed in reversed order of scheduling times.
 *
 *  See \ref impl::threadpool for common info.
 */
template <typename job>
class threadpool_LIFO: public impl::threadpool<impl::queue_LIFO, job> {};

}  // end of namespace mt

#endif  // end of #ifndef mt__thread_hxx
