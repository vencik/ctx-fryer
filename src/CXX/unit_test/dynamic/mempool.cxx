/**
 *  \brief  Memory (object) pool unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/07/24
 *
 *  Legal notices
 *
 *  Copyright 2014 Vaclav Krpec
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
#include "dynamic/mempool.hxx"
#include "mt/barrier.hxx"
#include "mt/thread.hxx"

#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <list>


/** dynamic::objpool test thread routine forward declaration */
class objpool_test_routine;

/** dynamic::objectpool test thread */
typedef mt::thread<objpool_test_routine> objpool_test_thread_t;

/** dynamic::objpool test thread routine */
class objpool_test_routine {
    public:

    /** Configuration of the objpool test */
    struct config {
        /** Count of allocations to be done (per thread) */
        const unsigned allocation_count;

        /** Size of allocated object */
        const size_t allocation_size;

        dynamic::objpool pool;        /**< Object pool           */
        mt::barrier      sync_point;  /**< Synchronisation point */

        /**
         *  \brief  Constructor
         *
         *  \param  thread_cnt    Thread count
         *  \param  alloc_cnt     Number of allocations (per thread)
         *  \param  alloc_size    Size of allocated object
         *  \param  obj_prealloc  Number of objects pre-allocated in pool
         *  \param  obj_plimit    Limit on number of pooled objects
         */
        config(
            unsigned thread_cnt,
            unsigned alloc_cnt,
            size_t   alloc_size,
            size_t   obj_prealloc,
            size_t   obj_plimit)
        :
            allocation_count(alloc_cnt),
            allocation_size(alloc_size),
            pool(alloc_size, obj_prealloc, obj_plimit),
            sync_point(thread_cnt)
        {}

    };  // end of struct config

    typedef config & arg_t;

    private:

    std::string m_msg_prefix;  /**< Log message prefix */

    public:

    /** Set log message prefix */
    void set_msg_prefix(objpool_test_thread_t & self) {
        std::stringstream ss;
        ss << "objpool test thread " << std::hex << self.id() << ": ";

        m_msg_prefix = ss.str();
    }

    /** Log message */
    void log(const std::string & msg) const {
        std::cerr << m_msg_prefix << msg << std::endl;
    }

    /** Implementation */
    int operator () (objpool_test_thread_t & self) {
        int  exit_code = 0;
        bool selected;

        set_msg_prefix(self);

        log("STARTED");

        config & conf = self.routine_argument();

        log("Syncing for objpool test");

        selected = conf.sync_point.wait();

        for (unsigned i = 0; i < conf.allocation_count; ++i) {
            void * obj = conf.pool.lim_alloc();

            if (NULL != obj)
                conf.pool.free(obj);
        }

        if (selected)
            log("(selected on sync)");

        log("Syncing for system allocator test");

        selected = conf.sync_point.wait();

        size_t size = conf.pool.size();

        for (unsigned i = 0; i < conf.allocation_count; ++i) {
            char * obj = new char[size];

            if (NULL != obj)
                delete obj;
        }

        if (selected)
            log("(Selected on sync)");

        log("FINISHED");

        return exit_code;
    }

};  // end of class objpool_test_routine

/** dynamic::objpool unit test */
int test_objpool() {
    unsigned thread_cnt   = 8;
    unsigned alloc_cnt    = 100000;
    size_t   alloc_size   = 83;
    size_t   obj_prealloc = 10;
    size_t   obj_plimit   = 321;

    objpool_test_routine::config config(
        thread_cnt,
        alloc_cnt,
        alloc_size,
        obj_prealloc,
        obj_plimit);

#ifdef HAVE_CXX11
    // Start test threads
    std::list<objpool_test_thread_t> threads;

    for (unsigned i = 0; i < thread_cnt; ++i)
        threads.emplace_back(config);

    // Join test threads
    std::list<objpool_test_thread_t>::iterator i = threads.begin();

    for (; i != threads.end(); ++i)
        i->wait();

#else  // Can't emplace threads to list

    // Since threads can't be copied, start fixed number of threads
    std::cerr
        << "WARNING: only using 5 objpool test threads (no C++11)"
        << std::endl;

    objpool_test_thread_t tt1(config);
    objpool_test_thread_t tt2(config);
    objpool_test_thread_t tt3(config);
    objpool_test_thread_t tt4(config);
    objpool_test_thread_t tt5(config);

    // Join test threads
    tt1.wait();
    tt2.wait();
    tt3.wait();
    tt4.wait();
    tt5.wait();
#endif  // end of #ifdef HAVE_CXX11

    return 0;
}


/** Memory & object pool unit tests */
int main_impl(int argc, char * const argv[]) {
    int exit_code = 0;

    // Test objpool
    exit_code = test_objpool();

    if (0 != exit_code) return exit_code;

    return 0;  // all tests passed
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
