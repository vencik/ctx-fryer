/**
 *  \brief  Logging
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2015/01/06
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


#include "proc/log.hxx"
#include "sys/time.hxx"

#include <sstream>
#include <cassert>
#include <ctime>
#include <cstdio>
#include <cstring>

extern "C" {
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <aio.h>
}


namespace proc {

namespace impl {

/**
 *  \brief  Integer -> string
 *
 *  \param  i  An integer
 *
 *  \return String version of the integer
 */
static std::string itoa(long long i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}

}  // end of namespace impl


// logger_fe members

const              pid_t logger_fe::s_pid(::getpid());
const thread_local pid_t logger_fe::s_tid(syscall(SYS_gettid));

const std::string logger_fe::s_pid_str(
        impl::itoa(logger_fe::s_pid));
const thread_local std::string logger_fe::s_tid_str(
        impl::itoa(logger_fe::s_tid));


// Assemble log line
std::pair<char *, size_t> logger_fe::log_line(
    logger::level_t     lvl,
    const char *        file,
    const char *        line,
    const char *        func,
    const std::string & msg)
{
    static const char * level_str[] = {
        /* ALWAYS */  "(**)",
        /* FATAL  */  "(!!)",
        /* ERROR  */  "(EE)",
        /* WARN   */  "(WW)",
        /* INFO   */  "(II)",
        /* DEBUG  */  "(DD)",  // == DEBUG0
        /* DEBUG1 */  "(D1)",
        /* DEBUG2 */  "(D2)",
        /* DEBUG3 */  "(D3)",
        /* DEBUG4 */  "(D4)",
        /* DEBUG5 */  "(D5)",
        /* DEBUG6 */  "(D6)",
        /* DEBUG7 */  "(D7)",
        /* DEBUG8 */  "(D8)",
        /* DEBUG9 */  "(D9)",
    };  // end of log level strings

    assert(logger::ALWAYS <= lvl);
    assert(logger::DEBUG9 >= lvl);

    size_t len = 0;

    // Log level string as in "(EE) "
    len += 5;

    // Identification, as in "<PID>.<TID>[.<LoggerID>] "
    len += s_pid_str.size() + s_tid_str.size() + 2;
    if (!m_id.empty()) len += m_id.size() + 1;

    // Current time as in "on 2015/01/06 12:30:45.123456 "
    sys::timer::time now = sys::timer::current_time(
        m_gmt ? sys::timer::GMT : sys::timer::localtime);
    len += 30;

    // Function as in "in main "
    len += ::strlen(func) + 4;

    // Source position as in "at foo.cxx:321"
    len += ::strlen(file) + ::strlen(line) + 4;

    // Message as in ": <msg>"
    len += msg.size() + 2;

    // New line
    ++len;

    // Assemble log line
    char * log_line = new char[len + 1];  // one more char for terminator

    int cnt = ::snprintf(log_line, len + 1,
        /* Log level  */  "%s "
        /* Identifier */  "%s.%s%s%s "
        /* Time       */  "on %04d/%02d/%02d %02d:%02d:%02d.%-6d "
        /* Function   */  "in %s "
        /* Position   */  "at %s:%s"
        /* Message    */  ": %s"
        /* New line   */  "\n",

        // Log level
        level_str[lvl],

        // Identifier, i.e. PID.TID[.LID]
        s_pid_str.c_str(), s_tid_str.c_str(),
        (m_id.empty() ? "" : "."), m_id.c_str(),

        // Time
        now.year, now.month,  now.day,
        now.hour, now.minute, now.second,
        now.nsec / 1000,

        // Function
        func,

        // Position
        file, line,

        // Message
        msg.c_str());

    if (cnt > len + 1)
        throw std::logic_error(
            "proc::logger_fe: line buffer too short");

    return std::pair<char *, size_t>(log_line, len);
}


// file_logger_be::aio_slots members

// Schedule I/O operation
void file_logger_be::aio_slots::schedule(worker_args & args) {
    enqueue_write(impl[begin++], args);

    begin %= PROC__LOG__MAX_PARALLEL_AIO_OPS;

    ++cnt;
}


bool file_logger_be::aio_slots::complete() {
    int i = PROC__LOG__MAX_PARALLEL_AIO_OPS - cnt;  // # of vacant

    // Index of the oldest pending operation control block
    i = (begin + i) % PROC__LOG__MAX_PARALLEL_AIO_OPS;

    int diff = complete_write(impl + i);

    cnt -= diff;

    return diff > 0;
}


// file_logger_be members

file_logger_be::aio_slots file_logger_be::s_slots;
file_logger_be::worker_t  file_logger_be::s_worker(false);  // start manually
file_logger_be::queue_t   file_logger_be::s_queue;
mt::mutex                 file_logger_be::s_mutex;
mt::condition             file_logger_be::s_qfull;


// Constructor
file_logger_be::file_logger_be(const std::string & logfile):
    log      ( logfile.empty() ? "/dev/stderr" : logfile ),
    m_log_fd ( -1 )
{
    open();
}


// Open log file
void file_logger_be::open() {
    // Already opened
    if (0 <= m_log_fd) return;

    // Standard FDs
    if ("/dev/stdout" == log)
        m_log_fd = STDOUT_FILENO;
    else if ("/dev/stderr" == log)
        m_log_fd = STDERR_FILENO;

    // Open file
    else {
        m_log_fd = ::open(log.c_str(),
            O_WRONLY | O_CREAT | O_APPEND,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

        if (0 > m_log_fd)
            throw std::runtime_error(
                "proc::file_logger_be: failed to open log file");
    }
}


// Close log file
void file_logger_be::close() {
    // Already closed
    if (0 > m_log_fd) return;

    // Won't close standard FDs
    if (STDOUT_FILENO == m_log_fd) return;
    if (STDERR_FILENO == m_log_fd) return;

    // Flush buffered data
    if (0 != ::fsync(m_log_fd))
        throw std::runtime_error(
            "proc::file_logger_be: failed to flush log file");

    // Close file
    if (0 != ::close(m_log_fd))
        throw std::runtime_error(
            "proc::file_logger_be: failed to close log file");

    m_log_fd = -1;
}


// Enqueue message buffer (implementation)
void file_logger_be::enqueue_msg(int fd, char * buffer, size_t length) {
    lock4scope(s_mutex);

    s_queue.push(fd, buffer, length);
    s_qfull.signal();
}


// Write buffer to log file asynchronously
void file_logger_be::enqueue_write(
    struct aiocb & cb,
    worker_args  & args)
{
    ::memset(&cb, 0, sizeof(cb));

    // Overtake arguments
    cb.aio_fildes = args.fd;
    cb.aio_buf    = args.buffer;
    cb.aio_nbytes = args.length;

    args.buffer = NULL;

    // Additional AIO settings
    //cb.aio_offset   = 0;  // TODO: do we need to shift the offset?
    //cb.aio_reqprio  = 0;  // TODO: guess 0 is OK...

    cb.aio_sigevent.sigev_notify = SIGEV_NONE;

    // Initiate the operation
    if (aio_write(&cb))
        throw std::runtime_error(
            "proc::file_logger_be: failed to enqueue buffer");
}


// Complete writing of a buffer to log file
int file_logger_be::complete_write(struct aiocb * cb)
{
    int status = ::aio_error(cb);

    switch (status) {
        case EINPROGRESS: return 0;  // not done yet

        case 0:          // done
        case ECANCELED:  // canceled
            delete[] (char *)cb->aio_buf;
            return 1;

        default:  // error
            // TODO: We probably shouldn't tear things down
            // in case of I/O error...
            // Should be put in #ifdef DEBUG cond. comp. block or something
#if (0)
            fprintf(stderr, "proc::file_logger_be: "
                    "failed to write buffer to fd %d: %d: %s",
                    cb->aio_fildes, status, strerror(status));
#endif

            throw std::runtime_error(
                "proc::file_logger_be: failed to write buffer");
    }
}


// Worker main routine
void file_logger_be::worker_main() {
    // In case the worker needs to wait for finalisation of some
    // I/O operations, this is the minimal latency (1 ms)
    static const struct timespec latency = { 0, 1000000 };

    lock4scope(s_mutex);

    for (;;) {
        // Message waits in queue
        if (!s_queue.empty()) {
            // No available I/O slot
            if (!s_slots.available()) {
                unlock4scope(s_mutex);

                // Attempt to complete I/O operation
                while (!s_slots.complete()) {
                    // Still no available I/O slot (overload condition),
                    // allow some time for I/O operations completion
                    ::nanosleep(&latency, NULL);
                }
            }

            assert(s_slots.available());

            // Pop message from queue
            worker_args args;
            s_queue.pop(args);

            // Stop signal
            if (0 > args.fd) break;

            // Schedule writing to log
            unlock4scope(s_mutex);
            s_slots.schedule(args);
            continue;  // next message
        }

        // No message in queue, pending I/O operations
        if (s_slots.pending()) {
            // Wait for messages while allowing some time
            // for the pending I/O operations to finish
            bool qfull = s_qfull.wait(s_mutex, 0.1);

            // Complete I/O operations after timeout
            // (it's likely lot of them were finished)
            if (!qfull) {
                unlock4scope(s_mutex);

                do if (!s_slots.complete()) break;
                while (s_slots.pending());
            }
        }

        // No messages in queue, no pending I/O operations
        else {
            // Wait for message
            s_qfull.wait(s_mutex);
        }
    }

    // Finalise pending I/O operations
    while (s_slots.pending()) {
        if (!s_slots.complete()) {
            // Pending I/O operations need more time to finish
            ::nanosleep(&latency, NULL);
        }
    }
}


// Start I/O worker
void file_logger_be::start_worker() {
    s_worker.start();
}


// Stop I/O worker
void file_logger_be::stop_worker() {
    // Don't bother unless the I/O worker was started
    if (decltype(s_worker)::STATUS_STARTUP > s_worker.status()) return;

    enqueue_msg(-1, NULL, 0);
    s_worker.join();
}


// file_logger members

// Constructor
file_logger::file_logger() {
    start();
}


// Constructor
file_logger::file_logger(
    const std::string & id,
    const std::string & file,
    logger::level_t     level,
    bool                gmt,
    bool                do_start)
:
    m_fe(id, level, gmt),
    m_be(file)
{
    if (do_start) start();
}


// Log message
void file_logger::message(
    logger::level_t     lvl,
    const char *        file,
    const char *        line,
    const char *        func,
    const std::string & msg)
{
    auto log_line = m_fe.log_line(lvl, file, line, func, msg);
    m_be.enqueue(log_line.first, log_line.second);
}


// Destructor
file_logger::~file_logger() {
    stop();
}

}  // end of namespace proc
