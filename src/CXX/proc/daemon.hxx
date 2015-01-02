#ifndef proc__daemon_hxx
#define proc__daemon_hxx

/**
 *  \brief  Daemon process
 *
 *  Support for turning into daemon (aka system service).
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


#include <string>


namespace proc {

/**
 *  \brief  Daemon process
 *
 *  Creation of the daemon means that the current process passes
 *  control to a newly created process that runs on backgroud, has the init
 *  process as its parent and doesn't have any further dependency on current
 *  session.
 *  The current process terminates with exit code of 0.
 *
 *  Note that the lifespan of the daemon object is indeed the lifespan of
 *  the process; ~daemon destructor terminates the process.
 *  This is because the destructor proforns certain complementary actions
 *  (like removal of the PID file) that conclude logical existence of the
 *  daemon and it would not be correct to do any processing afterwards.
 */
class daemon {
    private:

    const char * m_pid_file;   /**< PID file */

    public:

    int exit_code;  /**< Daemon exit code (0 by default) */

    /**
     *  \brief  Default constructor (no PID file)
     *
     *  Throws an exception if it failed start the daemon.
     */
    daemon();

    /**
     *  \brief  Constructor
     *
     *  Throws an exception if it failed start the daemon.
     *
     *  \param  pid_file  Path to file where the daemon PID shoud be stored
     */
    daemon(const std::string & pid_file);

    /**
     *  \brief  Destructor
     *
     *  Removes PID file (if any) and terminates with \ref exit_code.
     */
    ~daemon();

};  // end of class daemon

}  // end of namespace proc

#endif  // end of #ifndef proc__daemon_hxx
