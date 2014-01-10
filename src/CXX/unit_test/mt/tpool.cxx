#include "mt/thread.hxx"
#include "mt/semaphore.hxx"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <ctime>


/** Job script */
class job_script {
    private:

    /** Action */
    class action {
        public:

        enum type_t {
            ACTION_NOOP,   /**< No operation (yet)                 */
            ACTION_PRINT,  /**< Print a message                    */
            ACTION_SLEEP,  /**< Suspend execution for defined time */
        };  // end of enum type_t

        private:

        typedef std::list<std::string> arg_list_t;  /**< Action argument list */

        type_t     m_type;  /**< Action type      */
        arg_list_t m_args;  /**< Action arguments */

        /** Print action implementation */
        void print() const {
            arg_list_t::const_iterator arg = m_args.begin();

            while (arg != m_args.end()) {
                std::cout << *arg;

                ++arg;

                if (arg == m_args.end()) break;

                std::cout << ' ';
            }

            std::cout << std::endl;
        }

        /** Sleep action implementation */
        void sleep() const {
            if (1 != m_args.size())
                throw std::runtime_error("Invalid sleep action definition");

            // Get time specification (in seconds)
            double sec;
            std::stringstream sec_ss(m_args.front());
            if ((sec_ss >> sec).fail())
                throw std::runtime_error("Invalid sleep action argument");

            struct timespec ts;
            ts.tv_sec = (time_t)sec;
            sec -= ts.tv_sec;
            sec *= 1000000000;
            ts.tv_nsec = (long)sec;

            if (::nanosleep(&ts, NULL))
                throw std::runtime_error("Sleep action interrupted");
        }

        public:

        /** Constructor */
        action(): m_type(ACTION_NOOP) {}

        /** Type setter */
        inline void set_type(type_t type) { m_type = type; }

        /** Add argument */
        inline void add_arg(const std::string & arg) { m_args.push_back(arg); }

        /** Execute the action */
        void exec() const {
            switch (m_type) {
                case ACTION_NOOP:
                    // Another job well done ;-)
                    break;

                case ACTION_PRINT:
                    print();
                    break;

                case ACTION_SLEEP:
                    sleep();
                    break;
            }
        }

    };  // end of class action

    typedef std::list<action> action_list_t;  /**< List of actions */

    std::string   m_name;      /**< Job name                                 */
    int           m_priority;  /**< Job priority (only for priority queue TP */
    action_list_t m_actions;   /**< Job definition                           */

    public:

    /** Constructor */
    job_script(): m_priority(0) {}

    /** Comparison (by priority) */
    inline bool operator < (const job_script & rarg) const {
        return m_priority < rarg.m_priority;
    }

    /**
     *  \brief  Parse job script definition
     *
     *  \param  ins  Input stream
     *
     *  \retval -1 on EoF (empty job script)
     *  \retval  0 on success (job script parsed)
     *  \retval >0 on syntax error (grammar rule specific code)
     */
    int parse(std::istream & ins) {
        // Parsing state
        enum state_t {
            EXPECT_JOB,     // job [priority <P>] <name>
            EXPECT_ACTION,  //     <action> <argument>*
        };                  // endjob

        state_t state = EXPECT_JOB;

        std::string line;

        while (std::getline(ins, line)) {
            // Remove final (CR) LF
            if (line.size() && '\n' == line[line.size() - 1])
                line.erase(line.size() - 1);
            if (line.size() && '\r' == line[line.size() - 1])
                line.erase(line.size() - 1);

            // Remove leading & trailing spaces
            while (line.size() && (' ' == line[0] || '\t' == line[0]))
                line.erase(0, 1);
            while (line.size() && (' ' == line[line.size() - 1] || '\t' == line[line.size() - 1]))
                line.erase(line.size() - 1);

            // Skip empty lines & comments
            if (!line.size() || '#' == line[0]) continue;

            //std::cerr << "LINE: " << line << std::endl;

            // Resolve line
            switch (state) {
                case EXPECT_JOB: {
                    size_t space_pos = line.find(' ');
                    if (std::string::npos == space_pos)
                        space_pos = line.find('\t');

                    // Syntax error: the 1st line goes "job +(priority +\d+ +)?.*"
                    if (std::string::npos == space_pos)     return 1;
                    if ("job" != line.substr(0, space_pos)) return 2;

                    line.erase(0, 4);  // removing "job "

                    // Remove more eventual spaces
                    while (line.size() && (' ' == line[0] || '\t' == line[0]))
                        line.erase(0, 1);

                    // Optional priority specification
                    if (line.size() >= 8 && "priority" == line.substr(0, 8)) {
                        line.erase(0, 8);  // removing "priority"

                        // Priority specification must follow (after a space)
                        if (!line.size()) return 3;
                        if (' ' != line[0] && '\t' != line[0]) return 4;

                        do {
                            line.erase(0, 1);  // removing space
                        } while (line.size() && (' ' == line[0] || '\t' == line[0]));

                        space_pos = line.find(' ');
                        if (std::string::npos == space_pos)
                            space_pos = line.find('\t');

                        // Convert priority integer
                        std::stringstream prio_ss(line.substr(0, space_pos));
                        if ((prio_ss >> m_priority).fail()) return 5;

                        line.erase(0, space_pos);  // removing priority specification

                        // At least one space must follow
                        if (!line.size()) return 6;
                        if (' ' != line[0] && '\t' != line[0]) return 7;

                        line.erase(0, 1);  // removing the mandatory space

                        // Remove more eventual spaces
                        while (line.size() && (' ' == line[0] || '\t' == line[0]))
                            line.erase(0, 1);
                    }

                    m_name = line;

                    state = EXPECT_ACTION;

                    break;
                }

                case EXPECT_ACTION: {
                    action act;

                    // Job script ends
                    if ("endjob" == line) return 0;

                    size_t space_pos = line.find(' ');
                    if (std::string::npos == space_pos)
                        space_pos = line.find('\t');

                    // Resolve action
                    std::string action_str = line.substr(0, space_pos);

                    // print [<message>]
                    if ("print" == action_str) {
                        act.set_type(action::ACTION_PRINT);

                        // Remove "print *"
                        line.erase(0, 5);

                        while (line.size() && (' ' == line[0] || '\t' == line[0]))
                            line.erase(0, 1);

                        // Set printed string
                        if (line.size())
                            act.add_arg(line);
                    }

                    // sleep <duration [s]>
                    else if ("sleep" == action_str) {
                        act.set_type(action::ACTION_SLEEP);

                        // Remove "sleep *"
                        line.erase(0, 5);

                        while (line.size() && (' ' == line[0] || '\t' == line[0]))
                            line.erase(0, 1);

                        // Set printed string
                        if (line.size())
                            act.add_arg(line);
                        else
                            return 8;  // sleep argument is mandatory
                    }

                    // Syntax error: unknown action
                    else return 9;

                    // Add action to job action list
                    m_actions.push_back(act);

                    break;
                }
            }
        }

        return EXPECT_JOB == state ? -1 : 10;  // EoF
    }

    /** Execute script */
    void exec() const {
        action_list_t::const_iterator action = m_actions.begin();

        for (size_t action_no = 1; action != m_actions.end(); ++action, ++action_no) {
            std::cerr
                << "Executing job \"" << m_name
                << "\" action #" << action_no
                << std::endl;

            action->exec();
        }
    }

};  // end of class job_script


/**
 *  \brief  Test job
 *
 *  Note the non-standard usage of semaphore for jobs joining.
 */
class test_job {
    private:

    job_script m_script;  /**< Job definition */

    static mt::semaphore s_done;  /**< Jobs done semaphore */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  script  Job script
     */
    test_job(const job_script & script): m_script(script) {
        --s_done;

        std::cerr
            << "Test job " << this << " (for script "
            << &script << ") created"
            << std::endl;
    }

    /**
     *  \brief  Copy constructor
     *
     *  Note that the constructor is necessary to keep
     *  the semaphore aware of the instance count.
     *
     *  \param  orig  Original job
     */
    test_job(const test_job & orig): m_script(orig.m_script) {
        --s_done;

        std::cerr
            << "Test job " << this << " (copy of "
            << &orig << ") created"
            << std::endl;
    }

    /** Job comparison */
    inline bool operator < (const test_job & rarg) const {
        return m_script < rarg.m_script;
    }

    /** Job execution */
    inline void operator () () const { m_script.exec(); }

    /** Destructor */
    ~test_job() {
        std::cerr
            << "Test job " << this << " destroyed"
            << std::endl;

        ++s_done;
    }

    /** Wait for all instances destroyed */
    static void wait4all() { s_done.wait(); }

    private:

    /** Assigning is forbidden */
    void operator = (const test_job & orig) {}

};  // end of class test_job

// test_job static members initialisations
mt::semaphore test_job::s_done;


/**
 *  \brief  Threadpool test
 *
 *  Template parameter \c my_tpool specifies threadpool type.
 *
 *  \return 0 on test passed, non-zero otherwise
 */
template <class my_tpool>
int test_tpool() {
    my_tpool tpool;

    for (unsigned job_no = 1; ; ++job_no) {
        // Get another job script from input
        job_script jscript;  // normally, it should be dynamic to avoid copying

        int parse_st = jscript.parse(std::cin);

        if (-1 == parse_st) break;  // EoF

        if (parse_st) {
            std::cerr << "Job script parse error: " << parse_st << std::endl;
            throw std::runtime_error("job script parse error");
        }

        // Schedule job
        typename my_tpool::job_sched_t sched_type = tpool.run(test_job(jscript));

        std::cerr << "Job #" << job_no << " schedule type: ";

        switch (sched_type) {
            case my_tpool::JOB_SCHED_FAST:
                std::cerr << "JOB_SCHED_FAST";

                break;

            case my_tpool::JOB_SCHED_NEWTHREAD:
                std::cerr << "JOB_SCHED_NEWTHREAD";

                break;

            case my_tpool::JOB_SCHED_WAIT:
                std::cerr << "JOB_SCHED_WAIT";

                break;
        }

        std::cerr << std::endl;
    }

    // Wait for jobs to finish
    std::cerr << "Waiting for jobs" << std::endl;

    test_job::wait4all();

    std::cerr << "Jobs finished" << std::endl;

    return 0;
}


/**
 *  \brief  Usage
 *
 *  \param  outs  Output stream
 *  \param  proc  Process name
 */
void usage(std::ostream & outs, const std::string & proc) {
    outs
        << "Usage: " << proc << " [OPTIONS]" << std::endl
        << std::endl
        << "OPTIONS:" << std::endl
        << "    -h              Display help and exit"          << std::endl
        << "    -f              Test FIFO queue threadpool"     << std::endl
        << "    -l              Test LIFO queue threadpool"     << std::endl
        << "    -p              Test priority queue threadpool" << std::endl
        << std::endl;
}


/** Queue type */
enum queue_type_t {
    QTYPE_FIFO,  /**< FIFO     queue */
    QTYPE_LIFO,  /**< LIFO     queue */
    QTYPE_PRIO,  /**< Priority queue */
};  // end of enum queue_type_t


/**
 *  \brief  Main routine (implemantation)
 *
 *  The \ref main function wraps around this function (for several reasons).
 *
 *  \param  argc  Command line argument count
 *  \param  argv  Command line arguments
 *
 *  \return Process exit code (see \ref main)
 */
int main_impl(int argc, char * const argv[]) {
    queue_type_t qtype = QTYPE_FIFO;

    // Resolve options
    static const char *opts = "hflp";
    int opt;

    while (-1 != (opt = getopt(argc, argv, opts))) {
        switch (opt) {
            case 'h':
                usage(std::cout, *argv);
                return 0;

            case 'f':
                qtype = QTYPE_FIFO;
                break;

            case 'l':
                qtype = QTYPE_LIFO;
                break;

            case 'p':
                qtype = QTYPE_PRIO;
                break;

            default:  // Unknown option
                std::cerr << std::endl;
                usage(std::cerr, *argv);
                return 1;
        }
    }

    // Test thread pool implementation
    switch (qtype) {
        case QTYPE_FIFO:
            return test_tpool<mt::threadpool_FIFO<test_job> >();

        case QTYPE_LIFO:
            return test_tpool<mt::threadpool_LIFO<test_job> >();

        case QTYPE_PRIO:
            return test_tpool<mt::threadpool_priority<test_job> >();
    }

    throw std::logic_error("INTERNAL ERROR: Unimplemented queue type test");
}


/**
 *  \brief  Main routine wrapper
 *
 *  Ensures execution of all destructors.
 *  Catches all unhandled exceptions.
 *  Does not return (issues \c exit function with
 *  the \c main routine implementation return code as argument).
 *
 *  \param  argc  Command line argument count
 *  \param  argv  Command line arguments
 */
int main(int argc, char * const argv[]) {
    int exit_code = 127;  // exit code for unhandled exceptions

    try {
        exit_code = main_impl(argc, argv);
    }
    catch (std::exception & ex) {
        std::cerr
            << "Caught standard exception: "
            << ex.what()
            << std::endl;
    }
    catch (...) {
        std::cerr
            << "Caught an unknown exception"
            << std::endl;
    }

    ::exit(exit_code);
}
