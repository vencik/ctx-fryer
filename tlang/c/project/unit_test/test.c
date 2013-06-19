#include "parser.h"

#include "test.h"
#include "buffer.h"
#include "syntax_analyser.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>


/* Defaults */
#define DEFAULT_LOG_LEVEL LOG_LVL_ERROR  /**< Default log level */


/** Log level */
int log_level = DEFAULT_LOG_LEVEL;
/** \cond */
#define LOG_LEVEL log_level
/** \endcond */


static int    quiet           = 0;  /**< Non-null means no printout    */
static int    compact         = 0;  /**< Non-null means compact prnout */
static size_t stack_cap_limit = 0;  /**< PDA stack capacity limit      */


typedef struct test_case test_case_t;  /**< Test case */


/** Test case */
struct test_case {
    char   *label;           /**< Label                  */
    int     accept;          /**< Accept flag            */
    char   *word;            /**< Word                   */
    size_t  word_len;        /**< Word length            */
    size_t *derivation;      /**< Derivation list        */
    size_t  derivation_len;  /**< Derivation list length */
};  /* end of struct test_case */


/*
 * Static functions declarations
 */

static int tcase_create(test_case_t *tcase, FILE *istream);

static int tcase_create_impl(test_case_t *tcase, FILE *istream);

static void tcase_destroy(test_case_t *tcase);

static void tcase_word_cleanup(void *tst_case, char *word, size_t word_len);

static int test_parser(const test_case_t *test_case, syxa_t *syxa);

static int exec_test_case(test_case_t *test_case);

static void usage(const char *this);


/*
 * Static functions definitions
 */

/**
 *  \brief  Test case constructor
 *
 *  The function is actually only an inline wrapper around
 *  \ref test_case_create_impl that does the test case creation.
 *  This one takes care of proper cleanup if the construction
 *  fails.
 *
 *  \param  tcase    Test case
 *  \param  istream  Input stream
 *
 *  \return See \ref test_case_create_impl
 */
inline static int tcase_create(test_case_t *tcase, FILE *istream) {
    /* Initialisation */
    tcase->label      = NULL;
    tcase->word       = NULL;
    tcase->derivation = NULL;

    int status = tcase_create_impl(tcase, istream);

    /* Cleanup on error */
    if (status)
        tcase_destroy(tcase);

    return status;
}


/**
 *  \brief  Test case constructor (the true thing)
 *
 *  \param  tcase    Test case
 *  \param  istream  Input stream
 *
 *  \retval 0       if test case was successfully created
 *  \retval EOF     if the input stream ends
 *  \retval EINVAL  in case of test case definition syntax error
 *  \retval ENOMEM  in case of memory error
 */
static int tcase_create_impl(test_case_t *tcase, FILE *istream) {
    assert(NULL != tcase);

    char    *line = NULL;
    size_t   line_size;
    ssize_t  line_len;

    /* Test case label */
    if (-1 == (line_len = getline(&line, &line_size, istream))) {
        /* Getline doesn't cleanup the buffer upon error */
        free(line);

        int erno = errno;

        if (erno) {
            ERROR("Failed to read test case label: %d: %s", erno, strerror(erno));

            return erno;
        }

        return EOF;
    }

    if ('\r' == line[line_len - 1]) line[--line_len] = '\0';
    if ('\n' == line[line_len - 1]) line[--line_len] = '\0';

    tcase->label = line;

    /* Read & parse test case info */
    char   accept;
    size_t word_len;
    int    derivation_len;
    int    scanf_status;

    scanf_status = fscanf(istream, " %c %zu %d\n",
                                   &accept, &word_len, &derivation_len);
    if (3 != scanf_status) {
        if (EOF == scanf_status) {
            int erno = errno;

            if (erno) {
                ERROR("Failed to parse test case info: %d: %s", erno, strerror(erno));

                return erno;
            }

            ERROR("Unexpected EoF while parsing test case info");
        }
        else
            ERROR("Test case info syntax error");

        return EINVAL;
    }

    if (-1 > derivation_len) {
        ERROR("Derivation length of %d is invalid", derivation_len);

        return EINVAL;
    }

    tcase->accept = accept == '*';

    /* Read the test case word */
    tcase->word = (char *)malloc(word_len * sizeof(char));

    if (NULL == tcase->word) {
        ERROR("Failed to allocate test case word space");

        return ENOMEM;
    }

    tcase->word_len = fread(tcase->word, sizeof(char), word_len, istream);

    if (tcase->word_len != word_len) {
        ERROR("Failed to read test case word");

        return EINVAL;
    }

    /* LF after the word */
    int lf = fgetc(istream);

    if ('\n' != lf) {
        if (EOF == lf)
            ERROR("Premature EoF after test case word");
        else
            ERROR("Trailing characters after test case word");

        return EINVAL;
    }

    /* Read derivation */
    if (-1 != derivation_len) {
        tcase->derivation_len = (size_t)derivation_len;

        if (derivation_len) {
            assert(0 < derivation_len);

            tcase->derivation = (size_t *)malloc(tcase->derivation_len * sizeof(size_t));

            if (NULL == tcase->derivation) {
                ERROR("Failed to allocate test case derivation list");

                return ENOMEM;
            }

            size_t i = 0;
            for (; i < tcase->derivation_len; ++i) {
                scanf_status = fscanf(istream, " %zu", tcase->derivation + i);

                if (1 != scanf_status) {
                    if (EOF == scanf_status) {
                        int erno = errno;

                        if (erno) {
                            ERROR("Failed to read test case derivation (item %zu): %d: %s",
                                  i + 1, erno, strerror(erno));

                            return erno;
                        }

                        ERROR("Unexpected EoF while parsing test case info");
                    }
                    else
                        ERROR("Test case info syntax error");

                    return EINVAL;
                }
            }
        }

        /* LF after derivation */
        lf = fgetc(istream);

        if ('\n' != lf) {
            if (EOF == lf)
                ERROR("Premature EoF after test case derivation");
            else
                ERROR("Trailing characters after test case derivation");

            return EINVAL;
        }
    }
    else {
        assert(NULL == tcase->derivation);

        tcase->derivation_len = 0;
    }

    return 0;
}


/**
 *  \brief  Test case destructor
 *
 *  \param  tcase  Test case
 */
static void tcase_destroy(test_case_t *tcase) {
    assert(NULL != tcase);

    free(tcase->label);
    free(tcase->word);
    free(tcase->derivation);
}


/**
 *  \brief  Cleanup routine for test case word
 *
 *  \param  tst_case  Test case
 *  \param  word      Test case word
 *  \param  word_len  Test case word length (not used)
 */
static void tcase_word_cleanup(void *tst_case, char *word, size_t word_len) {
    assert(NULL != tst_case);

    test_case_t *test_case = (test_case_t *)tst_case;

    assert(word == test_case->word);

    DEBUX("Cleaning test case %p word %p", test_case, word);

    free(test_case->word);
    test_case->word = NULL;
}


/**
 *  \brief  Perform syntax analyser test
 *
 *  Parse the test case word and compare the parsing result with the expected.
 *
 *  \param  test_case  Test case
 *  \param  syxa       Syntax analyser
 *
 *  \retval 0      on success
 *  \retval EINVAL if the the lexical analyser failed
 */
static int test_parser(const test_case_t *test_case, syxa_t *syxa) {
    int status = 0;

    sa_status_t parser_status = sa_parse(syxa);

    size_t  derivation_len;
    size_t *derivation = sa_derivation(syxa, &derivation_len);

    assert(NULL != derivation);

    /* Check derivation match */
    int derivation_match = 1;

    if (NULL != test_case->derivation) {
        derivation_match = derivation_len == test_case->derivation_len;

        size_t i = 0;
        for (; derivation_match && i < derivation_len; ++i)
            derivation_match = derivation[i] == test_case->derivation[i];

        if (!derivation_match) {
            ERROR("Derivation mismatch for \"%.*s\"",
                  (int)test_case->word_len, test_case->word);

            char *derivation_str;

            derivation_str = array2str(derivation, derivation_len,
                                       size_t, "%zu", " ");
            assert(NULL != derivation_str);

            ERROR("Derivation: %s", derivation_str);

            free(derivation_str);

            derivation_str = array2str(test_case->derivation,
                                       test_case->derivation_len,
                                       size_t, "%zu", " ");
            assert(NULL != derivation_str);

            ERROR("Should be:  %s", derivation_str);

            free(derivation_str);
        }
    }

    free(derivation);

    if (!derivation_match) return EINVAL;

    /* Check acceptance */
    switch(parser_status) {
        case SA_OK:
            /* Input accepted */
            if (sa_accept(syxa)) {
                DEBUG("The input was accepted by the parser");

                if (!test_case->accept) {
                    ERROR("Test case failed (word should've been rejected)");

                    status = EINVAL;
                }
            }

            /* ??? */
            else {
                ERROR("The parser reports success, yet the input isn't accepted");

                status = EINVAL;
            }

            break;

        case SA_INPUT_EXHAUSTED:
            ERROR("The parser reports input exhaustion");

            status = EINVAL;

            break;

        case SA_SYNTAX_ERROR:
            DEBUG("The input was rejected by the parser");

            if (test_case->accept) {
                ERROR("Test case failed (word should've been accepted)");

                status = EINVAL;
            }

            break;

        case SA_ERROR:
            ERROR("The parser failed (general error)");

            status = EINVAL;

            break;
    }

    return status;
}


/**
 *  \brief  Execute test case
 *
 *  Depending on the syntax analyser definition, the input
 *  line shall be parsed.
 *
 *  The function doesn't actually do the parsing itself.
 *  It's rather a wrapper that takes care of syntax analyser
 *  proper initialisation/finalisation.
 *
 *  \param  test_case  Test case
 *
 *  \retval 0        on success
 *  \retval non-zero in case of error
 */
static int exec_test_case(test_case_t *test_case) {
    assert(NULL != test_case);

    int status;

    /* Create syntax analyser */
    syxa_t syxa;

    if (NULL == parser_create(&syxa, 1, stack_cap_limit, 1, 1, 0, 1))
    {
        ERROR("Failed to create parser");

        return -1;
    }

    /* Push test case word */
    status = sa_add_data(&syxa, test_case->word, test_case->word_len, tcase_word_cleanup, test_case, 1);

    /* Failed to add data */
    if (SA_OK != status) {
        ERROR("Failed to push input line to the parser: %d", status);
    }

    /* Test syntax analyser */
    else {
        status = test_parser(test_case, &syxa);
    }

    /* Destroy syntax analyser */
    parser_destroy(&syxa);

    return status;
}


/**
 *  \brief  Usage
 *
 *  The function logs usage (no matter what's current log level).
 *
 *  \param  this  The binary name
 */
static void usage(const char *this) {
    LOG("Usage: %s [OPTIONS]", this);
    LOG("");
    LOG("OPTIONS:");
    LOG("    -h                  show this help and exit");
    LOG("    -q                  suppress printout");
    LOG("    -c                  compact printout (suitable for automatic testing)");
    LOG("    -s <stack limit>    PDA stack capacity limit (0 means none), default: %zu", stack_cap_limit);
    LOG("    -l <log level>      set log level, default: %d (%s)", DEFAULT_LOG_LEVEL, log_lvl2str(DEFAULT_LOG_LEVEL));
    LOG("    -S <RNG seed>       set random number generator seed");
    LOG("");
    LOG("Log level is an inverted threshold for message levels.");
    LOG("The lower it is, the less output is produced.");
    LOG("Messages with lower or equal level are allowed, level classes follow:");
    LOG("%d (%s) means that only inevitable messages are logged", LOG_LVL_ALL, log_lvl2str(LOG_LVL_ALL));
    LOG("%d (%s) allows fatal messages (process shall terminate)", LOG_LVL_FATAL, log_lvl2str(LOG_LVL_FATAL));
    LOG("%d (%s) allows errors (process may continue, anyway)", LOG_LVL_ERROR, log_lvl2str(LOG_LVL_ERROR));
    LOG("%d (%s) allows warnings (something's odd, but not necessarily wrong)", LOG_LVL_WARN, log_lvl2str(LOG_LVL_WARN));
    LOG("%d (%s) allows various user info messages", LOG_LVL_INFO, log_lvl2str(LOG_LVL_INFO));
    LOG("%d (%s) allows debugging info (for developer)", LOG_LVL_DEBUG, log_lvl2str(LOG_LVL_DEBUG));
    LOG("%d (%s) allows extended debugging info (lots of output)", LOG_LVL_DEBUX, log_lvl2str(LOG_LVL_DEBUX));
    LOG("");
    LOG("The random number generator is seeded by current time by default.");
    LOG("");
}


/**
 *  \brief  Main routine
 *
 *  \param  argc  Argument count
 *  \param  argv  Arguments
 *
 *  \retval 0        on success
 *  \retval non-zero otherwise
 */
int main(int argc, char * const argv[]) {
    unsigned int rng_seed = (unsigned int)time(NULL);

    int opt;

    while (-1 != (opt = getopt(argc, argv, "hqcs:l:S:"))) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                exit(0);

                break;

            case 'q':
                quiet = 1;

                break;

            case 'c':
                compact = 1;

                break;

            case 's':
                stack_cap_limit = (size_t)atoi(optarg);

                break;

            case 'l':
                log_level = atoi(optarg);

                break;

            case 'S':
                rng_seed = (unsigned int)atoi(optarg);

                break;

            default:
                FATAL("Failed to process options");

                usage(argv[0]);
                exit(1);
        }
    }

    if (optind < argc) {
        FATAL("Trailing arguments");

        usage(argv[0]);
        exit(1);
    }

    /* Seed RNG */
    /* Always log this so that a failed test may be re-run */
    srand(rng_seed);
    LOG("RNG seed: %u", rng_seed);

    test_case_t test_case;

    while (0 == tcase_create(&test_case, stdin)) {
        /* Run test case */
        TEST_CASE(test_case.label, exec_test_case, &test_case);

        tcase_destroy(&test_case);
    }

    return 0;
}
