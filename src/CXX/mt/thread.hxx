#ifndef mt__thread_hxx
#define mt__thread_hxx

#include "mt/mutex.hxx"
#include "mt/condition.hxx"

#include "dynamic/pointer.hxx"

#include <cassert>
#include <cstdlib>

export "C" {
#include <pthread.h>
}


namespace mt {

namespace impl {

/** POSIX thread wrapper (abstract) */
class POSIX_thread {
    private:

    pthread_t m_impl;  /**< POSIX thread handle */

    protected:

    /**
     *  \brief  Thread routine interface
     *
     *  \param  arg  Routine argument (passed from \c pthread_create)
     *
     *  \return A pointer (to be returned by the POSIX thread main function)
     */
    virtual void * routine(void * arg) = 0;

    private:

    /** Constructor of thread-local mirror object */
    POSIX_thread(pthread_t impl): m_impl(impl) {}

    /**
     *  \brief  POSIX thread main routine
     *
     *  The static function is used as POSIX thread main function.
     *  It creates the thread object \c self mirror (thread-local object
     *  representing the thread) and executes its thread routine.
     *
     *  The routine is NOT exception-aware; exceptions are NOT caught.
     *
     *  \param  arg  Argument passed from \c pthread_create
     *
     *  \return Routine return pointer
     */
    static void * main(void * arg) {
        POSIX_thread self(pthread_self());

        return self.routine(arg);
    }

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
                    throw std::memory_error("POSIX thread attributes allocation failed");

                default:
                    throw std::runtime_error("POSIX thread attributes creation failed");
            }
        }

        /** Destructor */
        ~attr() {
            int status = pthread_attr_destroy(&impl);

            switch (status) {
                case 0: break;

                case ENOMEM:
                    throw std::memory_error("POSIX thread attributes deallocation failed");

                default:
                    throw std::runtime_error("POSIX thread attributes destruction failed");
            }
        }

        /** Detach state setter */
        void set_detach_state(detach_state_t state) throw(std::runtime_error) {
            int status = pthread_attr_setdetachstate(&impl, (int)state);

            if (status)
                throw std::runtime_error("failed to set POSIX thread detach state");
        }

        /** Contention scope setter */
        void set_scope(scope_t scope) throw(std::runtime_error) {
            int status = pthread_attr_setscope(&impl, (int)scope);

            if (status)
                throw std::runtime_error("failed to set POSIX thread scope");
        }

        private:

        /** Scheduling scheme setter */
        void set_sched_scheme(sched_scheme_t scheme) throw(std::runtime_error) {
            int status = pthread_attr_setinheritsched(&impl, (int)scheme);

            if (status)
                throw std::runtime_error("failed to set POSIX thread scheduling scheme");
        }

        public:

        /** Scheduling policy setter */
        void set_sched_policy(sched_policy_t policy) throw(std::runtime_error) {
            int status = pthread_attr_setschedpolicy(&impl, (int)policy);

            if (status)
                throw std::runtime_error("failed to set POSIX thread scheduling policy");

            set_sched_scheme(SCHED_SCHEME_EXPLICIT);
        }

        /** Scheduling priority setter */
        void set_sched_priority(int priority) throw(std::runtime_error) {
            struct sched_param sched_params;

            int status = pthread_attr_getschedparam(&impl, &sched_params);

            if (status)
                throw std::runtime_error("failed to get POSIX thread scheduling parameters");

            sched_params.sched_priority = priority;

            status = pthread_attr_setschedparam(&impl, &sched_params);

            if (status)
                throw std::runtime_error("failed to set POSIX thread scheduling priority");

            set_sched_scheme(SCHED_SCHEME_EXPLICIT);
        }

        /** Guard size setter */
        void set_guard_size(size_t size) throw(std::runtime_error) {
            int status = pthread_attr_setguardsize(&impl, size);

            if (status)
                throw std::runtime_error("failed to set POSIX thread guard size");
        }

        /** Stack size setter */
        void set_stack_size(size_t size) throw(std::runtime_error) {
            void * sp;

            int status = posix_memalign(&sp, sysconf(_SC_PAGESIZE), size);

            if (status)
                throw std::runtime_error("failed to align POSIX thread stack memory");

            status = pthread_attr_setstack(&impl, sp, size);

            if (status)
                throw std::runtime_error("failed to set POSIX thread stack");
        }

    };  // end of class attr

    /**
     *  \brief  Start POSIX thread
     *
     *  Throws \c std::memory_error on memory error or \c std::runtime_error if
     *  thread could not be started for other reasons.
     *
     *  \param  attrs  Thread attributes
     *  \param  arg    Thread routine argument (to be passed to \c pthread_create)
     */
    void start(const attr & attrs, void * arg) throw(std::memory_error, std::runtime_error) {
        status = pthread_create(&m_impl, &attrs.impl, &main, arg);

        switch (status) {
            case 0: break;

            case EAGAIN:
                throw std::runtime_error("system resources insufficient to create POSIX thread");

            case EINVAL:
                throw std::runtime_error("invalid POSIX thread attributes");

            case EPERM:
                throw std::runtime_error("POSIX thread creation denied");

            default:
                throw std::runtime_error("POSIX thread creation failed");
        }
    }

    public:

    /** Constructor (default attributes, no routine init argument) */
    POSIX_thread() { start(attr(), NULL); }

    /**
     *  \brief  Constructor (default attributes)
     *
     *  \param  arg  Thread routine argument
     */
    POSIX_thread(void * arg) { start(attr(), arg); }

    /**
     *  \brief  Constructor
     *
     *  \param  attrs  Thread attributes
     *  \param  arg    Thread routine argument
     */
    POSIX_thread(const attr & attrs, void * arg = NULL) { start(attrs, arg); }

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
                throw std::runtime_error("POSIX thread deadlock detected");

            case EINVAL:
                throw std::runtime_error("can't join POSIX thread");

            case ESRCH:
                throw std::runtime_error("POSIIX thread not found");

            default:
                throw std::runtime_error("failed to join POSIX thread");
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
            throw std::runtime_error("failed to cancel POSIX thread");
    }

    static const void * CANCELED;  /**< Canceled POSIX thread return value */

    /** Cancel thread and wait until it terminates */
    inline void cancel() const throw(std::runtime_error) {
        cancel();

        if (CANCELED != join())
            throw std::runtime_error("unexpected canceled POSIX thread return value");
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

        if (state)
            throw std::runtime_error("failed to set POSIX thread cancelation state");

        status = pthread_setcanceltype((int)type, &old);

        if (state)
            throw std::runtime_error("failed to set POSIX thread cancelation type");
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
    inline void detach() const throw(std::runtile_error) {
        int status = pthread_detach(m_impl);

        if (status)
            throw std::runtime_error("failed to detach POSIX thread");
    }

    /**
     *  \brief  Exit calling thread
     *
     *  \param  retval  Thread return value
     */
    static inline void exit(void * retval = NULL) throw() { pthread_exit(retval); }

};  // end of class POSIX_thread

// POSIX_thread static members initialisers
void * POSIX_thread::CANCELED = PTHREAD_CANCELED;


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
 *  the job queue.
 *  This (common) implementation only expects the following functions being
 *  defined by the queue:
 *
 *   bool empty();
 *   T pop();
 *
 *  The queue push operation may be highly specific; it is therefore not
 *  prototyped at all.
 *  For instance, priority queue requires the priority specification
 *  for push operation conversely to common FIFO queue.
 */
template <template <typename T> class queue_t>
class threadpool {
    friend class worker;  /**< Pooled worker thread friend */

    public:

    /** Job functor interface */
    class job {
        public:

        /** The job realisation */
        virtual void operator () () = 0;

        virtual ~job() {}  // mandatory virtual destructor

    };  // end of class job

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

    protected:

    typedef queue_t<job> job_queue_t;  /**< Job queue type definition */

    job_queue_t m_job_queue;  /**< Job queue */

    private:

    /**
     *  \brief  Routine performed by all workers
     *
     *  \param  worker
     */
    void worker_routine(worker & wt) {
        lock4scope(m_mutex);

        --m_prep;
        ++m_cnt;

        while (!m_shutdown) {
            // Check job queue
            if (!m_job_queue.empty()) {
                job my_job = m_job_queue.pop();

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

    /** Pooled worker tread */
    class worker: public impl::POSIX_thread {
        protected:

        /**
         *  \brief  Worker routine implementation
         *
         *  IMPORTANT NOTE:
         *  The routine is exception-aware; it doesn't allow propagation
         *  of any error to POSIX thread main function.
         *  Instead, it simply aborts the whole process.
         *
         *  \param  arg  Threadpool
         *
         *  \return \c NULL
         */
        void * routine(void * arg) {
            threadpool * tp = reinterpret_cast<threadpool *>(arg);

            try {
                tp->worker_routine(*this);
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

    };  // end of class worker

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
            worker new_worker(this);

            new_worker.detach();
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
        unsigned lo,
        unsigned hi,
        unsigned avail,
        double   ttl
    ):
        m_lo(lo), m_hi(hi),
        m_prep(0), m_avail(0), m_cnt(0),
        m_ttl(ttl),
        m_shutdown(false)
    {
        // Sanity checks
        if (!(lo <= hi))
            throw std::range_error("threadpool limits illegal");

        if (!(lo <= avail && avail <= hi))
            throw std::range_error("threadpool preallocation illegal");

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
    ~threadpool() {
        shutdown();
    }

    /**
     *  \brief  Push another job to job queue (i.e. schedule job)
     *
     *  \param  j  Job
     */
    inline void push(const job & j) {
        lock4scope(m_mutex);

        m_job_queue.push(j);
    }

};  // end of class threadpool

}  // end of namespace impl


/**
 *  \brief  Thread
 *
 *  The \c Routine template parameter should be a functor with
 *  parentheses operator accepting 2 arguments: the thread reference
 *  and another implementation-specific argument of type \c Routine::arg_t \c &
 *  and returns \c int as the thread exit code.
 */
template <class Routine>
class thread {
    public:

    enum status_t {
        STATUS_FAILED = -1,  /**< Thread failed to start */
        STATUS_INIT   =  0,  /**< Thread was initialised */
        STATUS_RUN,          /**< Thread routine runs    */
        STATUS_DONE,         /**< Thread has finished    */
        STATUS_DETACHED      /**< Thread is detached     */
    };  // end of enum status_t

    private:

    pthread_t     m_impl;    /**< POSIX thread             */
    status_t      m_status;  /**< Thread status            */
    int           m_xcode;   /**< Thread routine exit code */
    mutable mutex m_mutex;   /**< Operation mutex          */
    condition     m_st_ch;   /**< Status change condition  */

    public:

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
                throw std::logic_error("invalid thread exit");

            m_xcode  = xcode;
            m_status = STATUS_DONE;

            m_st_ch.broadcast();
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

            t->m_status = STATUS_RUN;

            t->m_st_ch.broadcast();
        }

        {   // Routine instance life is limited to this block
            Routine routine;

            xcode = routine(*t, t->m_arg);
        }

        t->exit(xcode);  // unless the routine has done it

        return NULL;  // unreachable code
    }

    public:

    /**
     *  \brief  Start thread
     *
     *  The function starts a new POSIX thread.
     *
     *  Note that the function throws an exception in case of an attempt
     *  to start the thread multiple times.
     *
     *  \param  arg  Thread routine argument
     *
     *  \return \c true if and only if the thread was started
     */
    bool start(const Routine::arg_t & arg) throw(std::logic_error, std::memory_error) {
        pthread_attr_t attr;

        int pt_st = pthread_attr_init(&attr);

        while (0 == pt_st) {  // pragmatic loop allowing for breaks
            pt_st = pthread_create(&m_impl, &attr, &main, this);

            if (0 != pt_st) break;

            pt_st = pthread_attr_destroy(&attr);  // TODO: process status (?)

            return true;
        }

        // Failed to start thread
        lock4scope(m_mutex);

        // Handle memory failure with exception
        if (ENOMEM == pt_st)
            throw std::memory_error("failed to init POSIX thread attributes");

        m_status = STATUS_FAILED;

        return false;
    }

    /**
     *  \brief  Create thread
     *
     *  \param  arg  Thread routine argument
     */
    thread():
        m_impl(0),
        m_status(STATUS_INIT),
        m_arg(arg),
        m_xcode(0) {}

    /**
     *  \brief  Create and start thread
     *
     *  \param  arg  Thread routine argument
     */
    thread(Routine::arg_t arg):
        m_impl(0),
        m_status(STATUS_INIT),
        m_arg(arg),
        m_xcode(0)
    {
        start();
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
            throw std::logic_error("thread routine didn't finish");

        return m_xcode;
    }

    /**
     *  \brief  Wait until thread finishes
     *
     *  \param  status  Status for which to wait (optional)
     *
     *  \return Thread status when done waiting
     */
    inline status_t wait(status_t status = STATUS_DONE) {
        lock4scope(m_mutex);

        if (STATUS_FAILED == m_status) return STATUS_FAILED;

        while (status > m_status)
            m_st_ch.wait(m_mutex);

        return m_status;
    }

    /**
     *  \brief  Wait until thread finishes
     *
     *  \param  timeout  Waiting timeout
     *  \param  status   Status for which to wait (optional)
     *
     *  \return Thread status when done waiting
     */
    inline status_t wait(double timeout, status_t status = STATUS_DONE) {
        lock4scope(m_mutex);

        if (STATUS_FAILED == m_status) return STATUS_FAILED;

        // Transitional phase, shouldn't take long
        while (STATUS_INIT == m_status)
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
    inline bool join() throw(std::logic_error) {
        lock4scope(m_mutex);

        if (STATUS_FAILED == m_status || STATUS_DETACHED == m_status)
            throw std::logic_error("invalid or double POSIX thread join");

        void * retval;
        int pt_st = pthread_join(&m_impl, &retval);

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
    inline bool detach() throw(std::logic_error) {
        lock4scope(m_mutex);

        if (STATUS_FAILED == m_status || STATUS_DETACHED == m_status)
            throw std::logic_error("invalid or double POSIX thread detachment");

        int pt_st = pthread_detach(&m_impl);

        if (0 != pt_st) return false;

        m_status = STATUS_DETACHED;

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
 *  See \ref impl::threadpool for common info.
 */
typedef impl::threadpool<queue> threadpool_FIFO;


/**
 *  \brief  Threadpool with priority job queue
 *
 *  See \ref impl::threadpool for common info.
 */
typedef impl::threadpool<pqueue> threadpool_pqueue;

}  // end of namespace mt

#endif  // end of #ifndef mt__thread_hxx
