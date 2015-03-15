/**
 *  \brief  Logger unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2015/03/15
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
#error "Sorry, C++11 support is required to compile this"
#endif

#include "proc/log.hxx"
#include "mt/thread.hxx"

#include <list>
#include <algorithm>
#include <cstdlib>


/** Logging worker */
class logging_worker {
    public:

    /** Thread routine argument type */
    typedef std::pair<proc::file_logger &, unsigned> arg_t;

    /** Logging worker thread */
    typedef mt::thread<logging_worker> thread_t;

    /**
     *  \brief  Worker routine
     *
     *  \param  thread  Thread object
     *
     *  \return Worker exit code
     */
    int operator () (thread_t & thread) {
        auto & logger = thread.routine_argument().first;
        #define LOGGER logger

        // Log messages
        unsigned msg_cnt = thread.routine_argument().second;
        for (unsigned i = 0; i < msg_cnt; ++i) {
            info("Worker " << thread.id() << " message " << i);
        }

        return 0;  // OK
    }

};  // end of class logging_worker


/**
 *  \brief  Logger unit test
 *
 *  \param  thread_cnt   Number of logging threads
 *  \param  message_cnt  Number of messages each thread logs
 *
 *  @return Exit code
 */
static int logger_test(
    unsigned thread_cnt,
    unsigned message_cnt)
{
    // Start logger
    proc::file_logger logger;
    logger.level(proc::logger::INFO);

    // Start logging threads
    std::list<logging_worker::thread_t> logging_threads;
    for (unsigned i = 0; i < thread_cnt; ++i)
        logging_threads.emplace_back(
            logging_worker::arg_t(logger, message_cnt));

    // Join logging threads
    std::for_each(logging_threads.begin(), logging_threads.end(),
        [](logging_worker::thread_t & thread) { thread.join(); });

    return 0;  // OK
}


/** Logger unit tests */
static int main_impl(int argc, char * const argv[]) {
    if (3 != argc) {
        std::cerr
            << "Usage: $0 <thread_cnt> <message_cnt>"
            << std::endl;

        return 1;
    }

    unsigned thread_cnt  = ::atoi(argv[1]);
    unsigned message_cnt = ::atoi(argv[2]);

    // Run logger UT
    return logger_test(thread_cnt, message_cnt);
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
