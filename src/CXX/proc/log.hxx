#ifndef proc__log_hxx
#define proc__log_hxx

/**
 *  \brief  Logging
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2015/01/05
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


#include "config.hxx"
#ifndef HAVE_CXX11
#error "Sorry, C++11 support required to compile this"
#endif

#include "mt/atomic.hxx"
#include "mt/thread.hxx"

#include <string>
#include <sstream>

extern "C" {
#include <aio.h>
}


/** Max. number of parallel async. I/O operations */
#define PROC__LOG__MAX_PARALLEL_AIO_OPS 64


#ifndef LOGGER_FUNCTION
/** Default function identifier form */
#define LOGGER_FUNCTION __func__
#endif

// Utility macra
/** \cond */
#define CONCAT_IMPL(a1, a2) a1 ## a2
#define CONCAT(a1, a2) CONCAT_IMPL(a1, a2)

#define STR_IMPL(a) #a
#define STR(a) STR_IMPL(a)
/** \endcond */

/**
 *  \brief  Logging macro (implementation)
 *
 *  The macro is resopnsible for checking the log level and if message
 *  log level priority is high enough, to assemble the message and
 *  pass it to logger.
 *
 *  Since the logging decision is done in the caller's context,
 *  the potentially time consuming and often unnecessary message assembly
 *  may be avoided.
 *
 *  \param  log   Logger
 *  \param  lvl   Log level
 *  \param  args  \c << concatenation of log message components
 */
#define proc__log__msg(log, lvl, args) \
    do { \
        if ((log).level() >= (lvl)) { \
            std::stringstream CONCAT(ss_, __LINE__); \
            CONCAT(ss_, __LINE__) << args; \
            (log).message((lvl), __FILE__, STR(__LINE__), \
            LOGGER_FUNCTION, CONCAT(ss_, __LINE__).str()); \
        } \
    } while (0)

// Logging macra
/** \cond */
#define always(args)  proc__log__msg((LOGGER), proc::logger::ALWAYS, args)
#define fatal(args)   proc__log__msg((LOGGER), proc::logger::FATAL,  args)
#define error(args)   proc__log__msg((LOGGER), proc::logger::ERROR,  args)
#define warn(args)    proc__log__msg((LOGGER), proc::logger::WARN,   args)
#define info(args)    proc__log__msg((LOGGER), proc::logger::INFO,   args)
#define debug(args)   proc__log__msg((LOGGER), proc::logger::DEBUG,  args)
#define debug0(args)  proc__log__msg((LOGGER), proc::logger::DEBUG0, args)
#define debug1(args)  proc__log__msg((LOGGER), proc::logger::DEBUG1, args)
#define debug2(args)  proc__log__msg((LOGGER), proc::logger::DEBUG2, args)
#define debug3(args)  proc__log__msg((LOGGER), proc::logger::DEBUG3, args)
#define debug4(args)  proc__log__msg((LOGGER), proc::logger::DEBUG4, args)
#define debug5(args)  proc__log__msg((LOGGER), proc::logger::DEBUG5, args)
#define debug6(args)  proc__log__msg((LOGGER), proc::logger::DEBUG6, args)
#define debug7(args)  proc__log__msg((LOGGER), proc::logger::DEBUG7, args)
#define debug8(args)  proc__log__msg((LOGGER), proc::logger::DEBUG8, args)
#define debug9(args)  proc__log__msg((LOGGER), proc::logger::DEBUG9, args)
/** \endcond */


/**
 *  \brief  Log & throw an exception
 *
 *  \param  lvl    Log level
 *  \param  xtype  Exception type
 *  \param  what   Exception message (const char *)
 */
#define proc_log_xthrow(lvl, xtype, what) \
    do { \
        proc__log__msg((LOGGER), (lvl), \
            "Throwing exception " STR(xtype) "(\"" << (what) << "\")"); \
        throw xtype(what); \
    } while (0)

// Throwing macra
/** \cond */
#define xthrow_fatal(xtype, what) \
    proc__log__xthrow(proc::logger::FATAL, xtype, what)
#define xthrow_error(xtype, what) \
    proc__log__xthrow(proc::logger::ERROR, xtype, what)

// By default, we consider exceptions to be indicating fatal flaws
#define xthrow(xtype, what) xthrow_fatal(xtype, what)
/** \endcond */

namespace proc {

/**
 *  \brief  Logger interface
 */
class logger {
    public:

    /** Log levels */
    enum level_t {
        ALWAYS = 0,  /** Logged regardless on the level                    */
        FATAL,       /** Fatal message, process shall terminate            */
        ERROR,       /** Error message (yet process may continue)          */
        WARN,        /** Warning (suspicious, but not ipso facto an error) */
        INFO,        /** Informative message                               */
        DEBUG,       /** Debugging message, highest priority               */

        // Log levels for fine-grained debugging
        DEBUG0 = DEBUG,  /** Debugging message, priority 0 */
        DEBUG1,          /** Debugging message, priority 1 */
        DEBUG2,          /** Debugging message, priority 2 */
        DEBUG3,          /** Debugging message, priority 3 */
        DEBUG4,          /** Debugging message, priority 4 */
        DEBUG5,          /** Debugging message, priority 5 */
        DEBUG6,          /** Debugging message, priority 6 */
        DEBUG7,          /** Debugging message, priority 7 */
        DEBUG8,          /** Debugging message, priority 8 */
        DEBUG9,          /** Debugging message, priority 9 */
    };

    /**
     *  \brief  Log message
     *
     *  \param  lvl   Message log level
     *  \param  file  Source file
     *  \param  line  Current line
     *  \param  func  Function identification
     *  \param  msg   Message
     */
    virtual void message(
        level_t             lvl,
        const char *        file,
        const char *        line,
        const char *        func,
        const std::string & msg) = 0;

};  // end of class logger


/**
 *  \brief  Common logging front-end
 *
 *  The front-end is expected to assemble the complete log line
 *  and pass it to back-end (responsible of actually putting it
 *  to the log implementation).
 *
 *  This separation of functionality makes sense namely in cases
 *  when multiple front-ends and/or back-ends (N:M mapping) are
 *  used, and also if the back-end is implemented as an async.
 *  separate thread, polling over log file(s) to unload the log I/O
 *  burden from logging workers...
 */
class logger_fe {
    private:

    static const              pid_t s_pid;  /** Process ID */
    static const thread_local pid_t s_tid;  /** Thread  ID */

    static const              std::string s_pid_str;  /**< PID string */
    static const thread_local std::string s_tid_str;  /**< TID string */

    const std::string m_id;     /**< Logger ID                       */
    mt::atomic_int    m_level;  /**< Log level                       */
    bool              m_gmt;    /**< Time in GMT (or else localtime) */

    public:

    /** Default constructor */
    logger_fe():
        m_level ( logger::ERROR ),
        m_gmt   ( true          )
    {}

    /**
     *  \brief  Constructor
     *
     *  \param  id     Logger ID
     *  \param  level  Initial log level
     *  \param  gmt    GMT timezone flag (localtime otherwise)
     */
    logger_fe(
        const std::string & id,
        logger::level_t     level = logger::ERROR,
        bool                gmt   = true)
    :
        m_id    ( id    ),
        m_level ( level ),
        m_gmt   ( gmt   )
    {}

    /** Log level getter */
    inline logger::level_t level() const {
        return (logger::level_t)((mt::atomic_int::local)m_level);
    }

    /** Log level setter */
    inline void level(logger::level_t lvl) {
        m_level = (mt::atomic_int::local)lvl;
    }

    /**
     *  \brief  Assemble log line
     *
     *  \param  lvl   Message log level
     *  \param  file  Source file
     *  \param  line  Current line
     *  \param  func  Function identification
     *  \param  msg   Message
     *
     *  \return Log line (including EoL)
     */
    std::pair<char *, size_t> log_line(
        logger::level_t     lvl,
        const char *        file,
        const char *        line,
        const char *        func,
        const std::string & msg);

};  // end of class logger_fe


/** File logger backend */
class file_logger_be {
    private:

    /** Worker task arguments */
    struct worker_args {
        int    fd;      /**< File descriptor       */
        char * buffer;  /**< Message buffer        */
        size_t length;  /**< Message buffer length */

        /** Default constructor */
        worker_args(): fd(-1), buffer(NULL), length(0) {}

        /** Constructor */
        worker_args(
            int    _fd,
            char * _buffer,
            size_t _length)
        :
            fd     ( _fd     ),
            buffer ( _buffer ),
            length ( _length )
        {}

        /** Move constructor */
        worker_args(worker_args && rval):
            fd     ( rval.fd     ),
            buffer ( rval.buffer ),
            length ( rval.length )
        {
            rval.buffer = NULL;
        }

        /** Rvalue assignment */
        inline worker_args & operator = (worker_args && rval) {
            if (NULL != buffer) delete buffer;

            fd     = rval.fd;
            buffer = rval.buffer;
            length = rval.length;

            rval.buffer = NULL;

            return *this;
        }

        /** Destructor */
        ~worker_args() {
            if (NULL != buffer) delete buffer;
        }

    };  // end of struct worker_args

    /** Buffer queue */
    typedef container::queue<worker_args> queue_t;

    /** AIO slots */
    struct aio_slots {
        /** AIO control blocks circular array */
        struct aiocb impl[PROC__LOG__MAX_PARALLEL_AIO_OPS];

        size_t begin;  /**< \c impl begin (1st vacant slot)  */
        size_t cnt;    /**< Number of pending operations     */

        /** Constructor */
        aio_slots():
            begin ( 0 ),
            cnt   ( 0 )
        {}

        /** Check if there is a slot available */
        inline bool available() const {
            return cnt < sizeof(impl) / sizeof(impl[0]);
        }

        /** Get number of pending operations */
        inline int pending() const { return cnt; }

        /**
         *  \brief  Schedule I/O operation
         *
         *  Note that the function overtakes the buffer from \c args.
         *
         *  \param  args  I/O operation arguments
         */
        void schedule(worker_args & args);

        /**
         *  \brief  Attempt to complete oldest I/O operation
         *
         *  \return \c true iff 
         */
        bool complete();

    };  // end of struct aio_slots

    /** I/O worker main routine (implementation) */
    static void worker_main();

    /** Log file I/O worker routine functor */
    class worker_routine {
        public:

        typedef int arg_t;  /**< Routine argument (not used) */

        inline int operator () (mt::thread<worker_routine> & thread) const {
            worker_main();

            return 0;
        }

    };  // end of class worker_routine

    /** Log file I/O worker */
    typedef mt::thread<worker_routine> worker_t;

    static aio_slots     s_slots;   /**< AIO slots           */
    static worker_t      s_worker;  /**< Log file I/O worker */
    static queue_t       s_queue;   /**< Buffer queue        */
    static mt::mutex     s_mutex;   /**< Operation mutex     */
    static mt::condition s_qfull;   /**< Queue has message   */

    /**
     *  \brief  Open log file
     *
     *  The function throws an exception if it fails to open the log file.
     */
    void open();

    /**
     *  \brief  Close log file
     *
     *  The function throws an exception if it fails to close the log file.
     */
    void close();

    /**
     *  \brief  Enqueue message buffer (implementation)
     *
     *  Passes the message buffer to the log file I/O worker.
     *
     *  \param  fd      File descriptor
     *  \param  buffer  Message buffer
     *  \param  length  Message buffer length
     */
    static void enqueue_msg(int fd, char * buffer, size_t length);

    /**
     *  \brief  Enqueue write operation
     *
     *  Note that the function overtakes the operation arguments.
     *
     *  \param  cb    Write operation control block
     *  \param  args  Write operation arguments
     */
    static void enqueue_write(struct aiocb & cb, worker_args & args);

    /**
     *  \brief  Complete writing of a buffer to log file
     *
     *  The function attempts to finalise the writing operation, i.e.
     *  if it was completed by kernel, it releases the buffer.
     *
     *  \param  cb  Write operation control block
     *
     *  \return 1 if operation was completed, 0 otherwise
     */
    static int complete_write(struct aiocb * cb);

    public:

    /** Start I/O worker */
    static void start_worker();

    /** Stop I/O worker */
    static void stop_worker();

    const std::string log;  /**< Log file */

    private:

    int m_log_fd;  /**< Log file descriptor */

    public:

    /** Default constructor (logs to STDERR) */
    file_logger_be();

    /**
     *  \brief  Constructor
     *
     *  \param  logfile  Log file path
     */
    file_logger_be(const std::string & logfile):
        log      ( logfile ),
        m_log_fd ( -1      )
    {
        open();
    }

    /**
     *  \brief  Enqueue message buffer
     *
     *  Passes the message buffer to the log file I/O worker.
     *
     *  \param  buffer  Message buffer
     *  \param  length  Message buffer length
     */
    inline void enqueue(char * buffer, size_t length) {
        enqueue_msg(m_log_fd, buffer, length);
    }

    /** Destructor */
    ~file_logger_be() { close(); }

    private:

    /** Copying is forbidden */
    file_logger_be(const file_logger_be & orig) = delete;

    /** Assignment is forbidden */
    void operator = (const file_logger_be & arg) = delete;

};  // end of class file_logger_be


/** File logger */
class file_logger: public logger {
    private:

    logger_fe      m_fe;  /**< Frontend */
    file_logger_be m_be;  /**< Backend  */

    public:

    /** Start backend */
    void start() { m_be.start_worker(); }

    /** Stop backend */
    void stop() { m_be.stop_worker(); }

    /**
     *  \brief  Default constructor
     *
     *  Logs to STDERR on ERROR level (initially), using GMP timestamps.
     */
    file_logger();

    /**
     *  \brief  Constructor
     *
     *  \param  id        Logger ID
     *  \param  file      Log file path
     *  \param  level     Initial log level
     *  \param  gmt       GMT timezone flag (localtime otherwise)
     *  \param  do_start  Whether to start the logger straight away
     */
    file_logger(
        const std::string & id,
        const std::string & file,
        logger::level_t     level    = logger::ERROR,
        bool                gmt      = true,
        bool                do_start = true);

    /** Log level getter */
    inline logger::level_t level() const { return m_fe.level(); }

    /** Log level setter */
    inline void level(logger::level_t l) { m_fe.level(l); }

    /**
     *  \brief  Log message
     *
     *  \param  lvl   Message log level
     *  \param  file  Source file
     *  \param  line  Current line
     *  \param  func  Function identification
     *  \param  msg   Message
     */
    void message(
        logger::level_t     lvl,
        const char *        file,
        const char *        line,
        const char *        func,
        const std::string & msg);

    /** Destructor (stops backend) */
    ~file_logger();

};  // end of class file_logger

}  // end of namespace proc

#endif  // end of #ifndef proc__log_hxx
