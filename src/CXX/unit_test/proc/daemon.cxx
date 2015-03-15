/**
 *  \brief  Daemon unit test
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

#include <iostream>
#include <string>
#include <cstdio>


/**
 *  \brief  Become daemon and wait for signal to terminate
 *
 *  \param  pid_file  PID file
 *  \param  sig_pipe  Signal pipe
 */
static void test_daemon(
    const std::string & pid_file,
    const std::string & sig_pipe)
{
    proc::daemon daemon(pid_file);

    daemon.exit_code = 32;  // pessimistic assumption

    FILE * pipe = ::fopen(sig_pipe.c_str(), "r");
    if (NULL == pipe) return;  // failed to open signal pipe

    // Wait for signal via the pipe
    ::fscanf(pipe, "\n");

    ::fclose(pipe);

    daemon.exit_code = 0;  // OK
}


/** Daemon unit tests */
static int main_impl(int argc, char * const argv[]) {
    if (3 != argc) {
        std::cerr
            << "Usage: $0 <pid_file> <sig_pipe>"
            << std::endl;

        return 1;
    }

    std::string pid_file(argv[1]);
    std::string sig_pipe(argv[2]);

    // Become daemon
    test_daemon(pid_file, sig_pipe);

    return 64;  // should never get here
}

/** Exception-safeness wrapper */
int main(int argc, char * const argv[]) {
    int exit_code = 127;  // exception caught

    try {
        exit_code = main_impl(argc, argv);
    }
    catch (std::exception & x) {
        std::cerr
            << "Standard exception caught: "
            << x.what()
            << std::endl;
    }
    catch (...) {
        std::cerr
            << "Unknown exception caught"
            << std::endl;
    }

    ::exit(exit_code);
}
