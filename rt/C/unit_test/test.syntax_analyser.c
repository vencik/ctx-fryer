/**
 *  \brief  Syntax analyser unit test
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2013/03/22
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2013 Vaclav Krpec
 *
 *  CTX Fryer C library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test.h"
#include "test.fsa_table.h"
#include "test.lexical_items.h"
#include "test.parser_tables.h"

#include "buffer.h"
#include "fsa.h"
#include "lexical_analyser.h"
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


static char   *line            = NULL;  /**< Input line                    */
static int     quiet           = 0;     /**< Non-null means no printout    */
static int     compact         = 0;     /**< Non-null means compact prnout */
static size_t  stack_cap_limit = 0;     /**< PDA stack capacity limit      */


/*
 * Static functions declarations
 */

static int parse_test(syxa_t *syxa);

static void line_cleanup(void *null, char *line, size_t size);

static int parse_line(const char *line, size_t line_len);

static void usage(const char *this);

static void finalise(void) __attribute__((destructor));


/**
 *  \brief  Perform syntax analysis test
 *
 *  Parse the input.
 *
 *  \param  syxa     Syntax analyser
 *
 *  \retval 0      on success
 *  \retval EINVAL if the the lexical analyser failed
 */
static int parse_test(syxa_t *syxa) {
    int status = 0;

    sa_status_t parser_status = sa_parse(syxa);

    size_t  derivation_len;
    size_t *derivation = sa_derivation(syxa, &derivation_len);

    assert(NULL != derivation);

    size_t i = 0;

    for (; i < derivation_len; ++i)
        fprintf(stdout, "%zu ", derivation[i]);

    switch(parser_status) {
        case SA_OK:
            /* Input accepted */
            if (sa_accept(syxa)) {
                INFO("The input was accepted by the parser");

                fprintf(stdout, ":-)");
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
            INFO("The input was rejected by the parser");

            fprintf(stdout, ":-(");

            break;

        case SA_ERROR:
            ERROR("The parser failed");

            status = EINVAL;

            break;
    }

    fprintf(stdout, "\n");

    free(derivation);

    /* Evaluate all parse root attributes */
    if (0 == status) {
        ptree_node_t *ptree_root = sa_ptree_ro(syxa);

        void *value = "value";

        attr_eval_status_t eval_status = ptree_node_attrs_eval(ptree_root,
            &value, NULL);

        if (ATTR_EVAL_OK == eval_status) {
            assert(NULL != value);

            INFO("The expression value is %g", *(double *)value);
        }
        else {
            ERROR("An attribute evaluation wasn't successful: %d", eval_status);
        }
    }

    return status;
}


/**
 *  \brief  Cleanup routine for input line
 *
 *  \param  null  Unused argument
 *  \param  line  Input line
 *  \param  size  Input line size
 */
static void line_cleanup(void *null, char *line, size_t size) {
    DEBUX("Cleaning input line %p", line);

    free(line);
}


/**
 *  \brief  Parse input line
 *
 *  Depending on the syntax analyser definition, the input
 *  line shall be parsed.
 *
 *  The function doesn't actually do the parsing itself.
 *  It's rather a wrapper that takes care of syntax analyser
 *  proper initialisation/finalisation.
 *
 *  \param  line      Input line
 *  \param  line_len  Length of the input line
 *
 *  \retval 0        on success
 *  \retval non-zero in case of error
 */
static int parse_line(const char *line, size_t line_len) {
    assert(NULL != line);

    int status;

    /* Create syntax analyser */
    syxa_t syxa;

    if (NULL == sa_create(&syxa,
                          &test_fsa, LEXICNT,
                          &test_action_tab, &test_goto_tab,
                          &test_rule_tab,   &test_attribute_tab,
                          1, stack_cap_limit, 1, 1, 0, 1))
    {
        ERROR("Failed to create parser");

        return -1;
    }

    /* Push line */
    char *line_copy = strdup(line);

    assert(NULL != line_copy);

    status = sa_add_data(&syxa, line_copy, line_len, line_cleanup, NULL, 1);

    /* Failed to add data */
    if (SA_OK != status) {
        ERROR("Failed to push input line to the parser: %d", status);
    }

    /* Test syntax analyser */
    else {
        status = parse_test(&syxa);
    }

    /* Destroy syntax analyser */
    sa_destroy(&syxa);

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

    size_t  line_size;
    ssize_t line_len;

    while (-1 != (line_len = getline(&line, &line_size, stdin))) {
        /* Remove EoL */
        if (line_len && '\n' == line[line_len - 1]) line[--line_len] = '\0';
        if (line_len && '\r' == line[line_len - 1]) line[--line_len] = '\0';

        /* Run test case */
        TEST_CASE(line, parse_line, line, line_len);

        free(line);
        line = NULL;
    }

    return 0;
}


/**
 *  \brief  Module destructor (called after \ref main)
 *
 *  Performs cleanup if necessary.
 *
 *  Note that attributed functions are GCC-specific.
 */
static void finalise(void) {
    free(line);
}
