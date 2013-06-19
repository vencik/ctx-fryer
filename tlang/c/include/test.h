#ifndef CTXFryer__test_h
#define CTXFryer__test_h

#include <stddef.h>
#include <stdio.h>


#define LOG_LVL_ALL    0  /**< God's   log level (always log)          */
#define LOG_LVL_FATAL  1  /**< Fatal   log level (termination's next)  */
#define LOG_LVL_ERROR  2  /**< Error   log level (things are bad)      */
#define LOG_LVL_WARN   3  /**< Warning log level (things are weird)    */
#define LOG_LVL_INFO   4  /**< Info    log level (things are OK)       */
#define LOG_LVL_DEBUG  5  /**< Debug   log level (devel. info)         */
#define LOG_LVL_DEBUX  6  /**< Debug^2 log level (excess. devel. info) */


/** Log levels string forms */
extern char * const test_log_levels[];

/** Unknown log level string form */
extern const char *test_log_level_unknown;


/**
 *  \brief  Resolve log level string
 *
 *  \param  lvl  Log level
 *
 *  \return Log level C-string (read-only)
 */
#define log_lvl2str(lvl) \
    (0 <= (lvl) && (lvl) <= LOG_LVL_DEBUX \
        ? test_log_levels[lvl] : test_log_level_unknown)


/**
 *  \brief  Resolve current log level string
 *
 *  \return Current log level C-string (read-only)
 */
#define LOG_LVL_STR log_lvl2str(LOG_LEVEL)


/**
 *  \brief  Logger
 *
 *  The macro expands to logging code including log level check.
 *  The current log level and the actual message level are compared
 *  and the message shall be logged if and only if its priority
 *  is high enough (i.e. if its level is numericaly less or equal
 *  to the log level).
 *  The macro logs position in the code, too.
 *
 *  Don't use this macro, directly; better use the following
 *  shortcuts (syntax is \c <SHRTCUT>(format, \c args \c ...)):
 *  \c LOGME (logs every time)
 *  \c FATAL (for messages that describe reasons why the process terminates)
 *  \c ERROR (for indications of erroneous states not necessarily fatal)
 *  \c WARN  (for warnings---not errors but somehow odd states etc)
 *  \c INFO  (for information about regular events)
 *  \c DEBUG (for development information)
 *  \c DEBUX (for excessive development information)
 *
 *  \param  log_level  Log level (#define LOG_LEVEL in your code)
 *  \param  msg_level  Message level (message priority)
 *  \param  format     Message format (see \c fprintf)
 *  \param  args       Message format arguments
 */
#define LOG_IMPL_POS(log_level, msg_level, format, args ...) \
    do { \
        if ((log_level) >= (msg_level) || LOG_LVL_ALL == (msg_level)) { \
            const char *msg_level_str = log_lvl2str(msg_level); \
            fprintf(stderr, "%s " format "\n", msg_level_str, ##args); \
            fprintf(stderr, "%s ... in %s at %s:%u\n", msg_level_str, __FUNCTION__, __FILE__, __LINE__); \
            fflush(stderr); \
        } \
    } while (0)


/**
 *  \brief  Logger
 *
 *  The macro logs just like \ref LOG_IMPL_POS, but omits the position.
 *
 *  \param  log_level  Log level (#define LOG_LEVEL in your code)
 *  \param  msg_level  Message level (message priority)
 *  \param  format     Message format (see \c fprintf)
 *  \param  args       Message format arguments
 */
#define LOG_IMPL(log_level, msg_level, format, args ...) \
    do { \
        if ((log_level) >= (msg_level) || LOG_LVL_ALL == (msg_level)) { \
            const char *msg_level_str = log_lvl2str(msg_level); \
            fprintf(stderr, "%s " format "\n", msg_level_str, ##args); \
            fflush(stderr); \
        } \
    } while (0)


/** \cond */
#define   LOG(format, args ...) LOG_IMPL(LOG_LEVEL, LOG_LVL_ALL,   format, ##args)
#define FATAL(format, args ...) LOG_IMPL(LOG_LEVEL, LOG_LVL_FATAL, format, ##args)
#define ERROR(format, args ...) LOG_IMPL(LOG_LEVEL, LOG_LVL_ERROR, format, ##args)
#define  WARN(format, args ...) LOG_IMPL(LOG_LEVEL, LOG_LVL_WARN,  format, ##args)
#define  INFO(format, args ...) LOG_IMPL(LOG_LEVEL, LOG_LVL_INFO,  format, ##args)

#define DEBUG(format, args ...) LOG_IMPL_POS(LOG_LEVEL, LOG_LVL_DEBUG, format, ##args)
#define DEBUX(format, args ...) LOG_IMPL_POS(LOG_LEVEL, LOG_LVL_DEBUX, format, ##args)
/** \endcond */


/**
 *  \brief  Execute test case
 *
 *  The macro expands to test function call and it's return
 *  value check which, if unsuccessful, is followed by
 *  error report and termination of process (with non-zero
 *  exit code).
 *  The test function is passed all the additional arguments
 *  of the macro and is expected to return integral status
 *  code (0 means that the test was successful, non-zero
 *  means an error).
 *
 *  \param  label     Test case label
 *  \param  function  Test case implementation
 *  \param  args      Test case implementation arguments
 */
#define TEST_CASE(label, function, args ...) \
    do { \
        INFO("Executing test case \"%s\" (function " #function ")", (label)); \
        int status = (function)(args); \
        if (0 != status) { \
            FATAL("Test case \"%s\" FAILED with status %d", (label), status); \
            exit(status); \
        } \
        INFO("Test case \"%s\" PASSED", (label)); \
    } while (0)


/**
 *  \brief  Format string form of an array of certain type
 *
 *  \param  array   Array
 *  \param  cnt     Count of array items
 *  \param  type    Type of items
 *  \param  format  \c printf format of one item
 *  \param  sep     Separator used in the string
 *
 *  \return Dynamic C-string containing the stringified array or \c NULL on error
 */
#define array2str(array, cnt, type, format, sep) \
    array2str_impl((array), (cnt), sizeof(type), (format), (sep), snprintf_ ## type)


/**
 *  \brief  Format \c size_t array item
 *
 *  \param  dest    Destination string
 *  \param  avail   Available size of the dest. string (including terminal '\0')
 *  \param  format  Required format string
 *  \param  item    Item formatted
 *
 *  \return \c snprintf return value (see \c snprintf man page)
 */
ssize_t snprintf_size_t(char *dest, size_t avail, const char *format, void *item);


/**
 *  \brief  Format string form of an array
 *
 *  \param  array           Array
 *  \param  cnt             Count of array items
 *  \param  size            Size of items
 *  \param  format          \c printf format of one item
 *  \param  sep             Separator used in the string
 *  \param  item_format_fn  Item formating function
 *
 *  \return Dynamic C-string containing the stringified array or \c NULL on error
 */
char *array2str_impl(void *array, size_t cnt, size_t size,
                     const char *format, const char *sep,
                     ssize_t (*item_format_impl)(char * , size_t , const char * , void * ));

#endif /* end of #ifndef CTXFryer__test_h */
