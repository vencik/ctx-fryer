/**
 *  \brief  Daemon process
 *
 *  Implementation.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2015/01/02
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
#include "proc/daemon.hxx"

#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <cassert>

extern "C" {
#include <sys/types.h>
#include <unistd.h>
}


namespace proc {

namespace impl {

/**
 *  \brief  Store current process PID to a file
 *
 *  \param  pid_file  Path to PID file
 *
 *  \return Failure reason or \c NULL in case of success
 */
static const char * store_pid(const char * pid_file) {
    assert(NULL != pid_file);

    // Open PID file
    FILE * pidf = ::fopen(pid_file, "w");
    if (NULL == pidf) return "PID file creation failure";

    // Store PID
    if (0 > ::fprintf(pidf, "%u", (unsigned)::getpid()))
        return "PID storage failure";

    // Close PID file
    if (0 != ::fclose(pidf)) return "failed to close PID file";

    return NULL;  // OK
}


/**
 *  \brief  Remove PID file
 *
 *  \param  pid_file  Path to PID file
 *
 *  \return Failure reason or \c NULL in case of success
 */
static const char * remove_pid(const char * pid_file) {
    assert(NULL != pid_file);

    if (0 != ::remove(pid_file))
        return "failed to remove PID file";

    return NULL;
}


/**
 *  \brief  Continue execution as daemon process (implementation)
 *
 *  \return Failure reason or \c NULL in case of success
 */
static const char * daemonise() {
    pid_t pid;

    // 1st fork to go to background
    pid = ::fork();
    if (0 >  pid) return "1st fork() failed";
    if (0 != pid) ::exit(0);  // terminate foregroud process

    // Create new session & become session leader
    if (0 > ::setsid()) return "setsid() failed";

    // 2nd fork to drop session leadership
    pid = ::fork();
    if (0 >  pid) return "2nd fork() failed";
    if (0 != pid) ::exit(0);  // terminate session leader

    // Close standard file descriptors
    if (0 > ::close(STDIN_FILENO))  return "close(STDIN) failed";
    if (0 > ::close(STDOUT_FILENO)) return "close(STDOUT) failed";
    if (0 > ::close(STDERR_FILENO)) return "close(STDERR) failed";

    // cd /
    if (0 > ::chdir("/")) return "chdir() failed";

    return NULL;
}


/**
 *  \brief  Throw exception wrapper for the above functions
 *
 *  The function throws an exception if te argument is not \c NULL.
 *
 *  \param  failure  Exception reason
 */
static void xthrow(const char * failure) throw(std::runtime_error) {
    if (NULL == failure) return;  // OK

    // Failure
    std::string xfailure("proc::daemon: ");
    xfailure += failure;

    throw std::runtime_error(xfailure);
}

}  // end of namespace impl


// daemon members

// Default constructor
daemon::daemon():
    m_pid_file ( NULL ),
    exit_code  ( 0    )
{
    impl::xthrow(impl::daemonise());
}

// Constructor
daemon::daemon(const std::string & pid_file):
    m_pid_file ( ::strdup(pid_file.c_str()) ),
    exit_code  ( 0                          )
{
    if (NULL == m_pid_file)
        impl::xthrow("failed to copy PID file path");

    impl::xthrow(impl::daemonise());
    impl::xthrow(impl::store_pid(m_pid_file));
}

// Destructor
daemon::~daemon() {
    if (NULL != m_pid_file) {
        impl::xthrow(impl::remove_pid(m_pid_file));

        ::free(const_cast<char *>(m_pid_file));
    }

    ::exit(exit_code);
}

}  // end of namespace proc
