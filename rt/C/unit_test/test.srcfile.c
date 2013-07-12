/**
 *  \brief  Source file reading unit test
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2012/08/02
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2012 Vaclav Krpec
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

#include "srcfile.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/** Page size multiplier default */
#define PAGE_SIZE_MUL_DEFAULT 1

/** Block size default */
#define BLOCK_SIZE_DEFAULT 1024


/** Log level */
int log_level = LOG_LVL_ERROR;
/** \cond */
#define LOG_LEVEL log_level
/** \endcond */


/** Dump source flag */
int dump_src = 0;


/**
 *  \brief  Call source file interface function (return on error)
 *
 *  The macro expands to call of source file interface function.
 *  Should the function return non-zero status code (\c errno),
 *  the macro logs the code and returns from current function
 *  using the very code as its return value.
 *
 *  \param  function  Interface function to be called
 *  \param  args      The \c function arguments
 */
#define SRCF_IFACE_RETURN_ON_ERROR(function, args ...) \
    do { \
        DEBUG("Calling iface function " #function); \
        int status = (function)(args); \
        if (status) { \
            ERROR("Iface function " #function " call failed: %d: %s", \
                  status, strerror(status)); \
            return status; \
        } \
        else { \
            DEBUX("Iface function " #function " call successful"); \
        } \
    } while (0)


/**
 *  \brief  Test source file module
 *
 *  \param  src_file  Source file
 *
 *  \retval 0        in case of success
 *  \retval non-zero otherwise
 */
int test(srcfile_t *src_file) {
    SRCF_IFACE_RETURN_ON_ERROR(srcfile_open, src_file);

    size_t buffer_cnt = 0;

    int done;

    do {
        buffer_t *buffer;

        SRCF_IFACE_RETURN_ON_ERROR(srcfile_next_buffer, src_file,
                                   &buffer, &done);
        ++buffer_cnt;

        DEBUG("Got buffer #%zu, size: %zu", buffer_cnt, buffer_size(buffer));

        /* Dump source */
        if (dump_src) {
            fprintf(stdout, "%.*s", (int)buffer_size(buffer), buffer_data(buffer));
            fflush(stdout);
        }

        buffer_destroy(buffer);

    } while (!done);

    INFO("Got %zu buffers", buffer_cnt);

    SRCF_IFACE_RETURN_ON_ERROR(srcfile_close, src_file);

    return 0;
}


/**
 *  \brief  Test source file in paged mode
 *
 *  The function is the source file object construction
 *  wrapper (for paged segmentation mode).
 *  It also makes sure that the destructor is called
 *  no matter what happens during the test (thus simuating
 *  C++ way of automatic object destruction upon leaving scope).
 *
 *  \param  src_file_name  Source file name
 *  \param  page_size_mul  Page size multiplier
 *
 *  \retval 0        in case of success
 *  \retval non-zero otherwise
 */
int test_page_mode(const char *src_file_name, size_t page_size_mul) {
    srcfile_t src_file;

    srcfile_create(&src_file, src_file_name, SRCFILE_SEG_PAGE, page_size_mul);

    int status = test(&src_file);

    srcfile_destroy(&src_file);

    return status;
}


/**
 *  \brief  Test source file in block mode
 *
 *  The function is the source file object construction
 *  wrapper (for block segmentation mode).
 *  It also makes sure that the destructor is called
 *  no matter what happens during the test (thus simuating
 *  C++ way of automatic object destruction upon leaving scope).
 *
 *  \param  src_file_name  Source file name
 *  \param  block_size     Block size
 *
 *  \retval 0        in case of success
 *  \retval non-zero otherwise
 */
int test_block_mode(const char *src_file_name, size_t block_size) {
    srcfile_t src_file;

    srcfile_create(&src_file, src_file_name, SRCFILE_SEG_BLOCK, block_size);

    int status = test(&src_file);

    srcfile_destroy(&src_file);

    return status;
}


/**
 *  \brief  Test source file in line mode
 *
 *  The function is the source file object construction
 *  wrapper (for line segmentation mode).
 *  It also makes sure that the destructor is called
 *  no matter what happens during the test (thus simuating
 *  C++ way of automatic object destruction upon leaving scope).
 *
 *  \param  src_file_name  Source file name
 *
 *  \retval 0        in case of success
 *  \retval non-zero otherwise
 */
int test_line_mode(const char *src_file_name) {
    srcfile_t src_file;

    srcfile_create(&src_file, src_file_name, SRCFILE_SEG_LINE);

    int status = test(&src_file);

    srcfile_destroy(&src_file);

    return status;
}


/**
 *  \brief  Usage
 *
 *  The function logs usage (no matter what's current log level).
 *
 *  \param  this  The binary name
 */
void usage(const char *this) {
    LOG("Usage: %s [OPTIONS] <source file>", this);
    LOG("");
    LOG("OPTIONS:");
    LOG("    -h                  show this help and exit");
    LOG("    -l <log level>      set log level, default: %d (%s)", LOG_LEVEL, LOG_LVL_STR);
    LOG("    -p <page size mul>  page size multiplier, default: %u", PAGE_SIZE_MUL_DEFAULT);
    LOG("    -b <block size>     block size, default: %u", BLOCK_SIZE_DEFAULT);
    LOG("    -d                  dump source to std. output");
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
    LOG("In paged mode, the source file is mapped to memory.");
    LOG("The page size is %zu; it's allowed to map by multiples of this size.", srcfile_page_size());
    LOG("");
    LOG("In block mode, buffers of const. size are read from the source file.");
    LOG("");
    LOG("In line mode, lines ended ");
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
    size_t page_size_mul = PAGE_SIZE_MUL_DEFAULT;
    size_t block_size    = BLOCK_SIZE_DEFAULT;

    int opt;

    while (-1 != (opt = getopt(argc, argv, "hl:p:b:d"))) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                exit(0);

                break;

            case 'l':
                log_level = atoi(optarg);

                break;

            case 'p':
                page_size_mul = (size_t)atol(optarg);

                break;

            case 'b':
                block_size = (size_t)atol(optarg);

                break;

            case 'd':
                dump_src = 1;

                break;

            default:
                FATAL("Failed to process options");

                usage(argv[0]);
                exit(1);
        }
    }

    if (optind >= argc) {
        FATAL("Source file not specified");

        usage(argv[0]);
        exit(1);
    }

    if (optind + 1 < argc) {
        FATAL("Trailing arguments after source file");

        usage(argv[0]);
        exit(1);
    }

    const char *src_file_name = argv[optind];

    /* Run test cases */
    TEST_CASE("Page mode",  test_page_mode,  src_file_name, page_size_mul);
    TEST_CASE("Block mode", test_block_mode, src_file_name, block_size);
    TEST_CASE("Line mode",  test_line_mode,  src_file_name);

    return 0;
}
