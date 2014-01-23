/**
 *  \brief  Lexical analyser unit test
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2012/09/05
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
#include "test.fsa_table.h"
#include "test.lexical_items.h"

#include "buffer.h"
#include "fsa.h"
#include "lexical_analyser.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>


/* Defaults */
#define DEFAULT_LOG_LEVEL   LOG_LVL_ERROR  /**< Default log level                       */
#define DEFAULT_MAX_SEG_CNT 33             /**< Default max. amount of segments         */
#define DEFAULT_MAX_SEG_ADD 5              /**< Max. amount of segments added at a time */


typedef struct segment     segment_t;      /**< Data segment          */
typedef struct segment_seq segment_seq_t;  /**< Data segment sequence */


/** Data segment state */
typedef enum {
    SEG_INITIALISED = 0,  /**< Segment was initialised */
    SEG_IN_USE,           /**< Segment is in use       */
    SEG_RELEASED,         /**< Segment was released    */
} segment_state_t;  /* end of typedef enum */


/** Data segment */
struct segment {
    char            *data;     /**< Segment data        */
    size_t           size;     /**< Segment size        */
    segment_state_t  state;    /**< Segment state       */
    unsigned int     use_cnt;  /**< Segment use counter */
};  /* end of struct segment */


/** Data segment sequence */
struct segment_seq {
    segment_t *impl;  /**< Array of segments */
    size_t     cnt;   /**< Segment count     */
};  /* end of struct segment_seq */


/**
 *  \brief  Get number of segments in sequence
 *
 *  \param  seg_seq  Segment sequence
 *
 *  \return Segment count
 */
#define segment_seq_cnt(seg_seq) ((seg_seq)->cnt)


/**
 *  \brief  Get segment data size
 *
 *  \param  seg  Segment
 *
 *  \return Segment data size
 */
#define segment_size(seg)  ((seg)->size)


/**
 *  \brief  Get segment state
 *
 *  \param  seg  Segment
 *
 *  \return Segment state
 */
#define segment_state(seg)  ((seg)->state)


/** Log level */
int log_level = DEFAULT_LOG_LEVEL;
/** \cond */
#define LOG_LEVEL log_level
/** \endcond */


static int   quiet       = 0;                      /**< Non-null means no printout    */
static int   compact     = 0;                      /**< Non-null means compact prnout */
static char *line        = NULL;                   /**< Line from input               */
static int   seg_max     = DEFAULT_MAX_SEG_CNT;    /**< Max. amount of segments       */
static int   seg_add_max = DEFAULT_MAX_SEG_ADD;    /**< Max. amount of added segments */


/*
 * Static functions declarations
 */

static int rand_from(int lo, int hi);
static int cmp_size_t(const void *size1, const void *size2);

static int segment_seq_create(segment_seq_t *seg_seq, char *data, size_t data_size,
                              const size_t offsets[], size_t seg_cnt);
static int segment_seq_create_rand(segment_seq_t *seg_seq, char *data, size_t data_size);
static int segment_seq_destroy(segment_seq_t *seg_seq);
static segment_t *segment_seq_at(segment_seq_t *seg_seq, size_t index);

static char *segment_use_data(segment_t *seg);
static void segment_drop_data(segment_t *seg);

static void buffer_over_segment_cleanup(void *segment, char *data, size_t size);
static void process_lexical_items(const la_item_t *items, size_t item_cnt);

static int segment_test(lexa_t *lexa, segment_seq_t *seg_seq);
static int segment_line(char *line, size_t line_len);

static void usage(const char *this);

static void finalise(void) __attribute__((destructor));


/**
 *  \brief  Random number from an interval
 *
 *  \param  lo  Lower bound (inclusive)
 *  \param  hi  Upper bound (inclusive)
 *
 *  \return Random number from [\c lo, \c hi]
 */
static int rand_from(int lo, int hi) {
    assert(lo <= hi);

    /* q \in [0,1] */
    double q = (double)rand() / RAND_MAX;

    double width = (double)(hi - lo);

    int result = lo + (int)(width * q);

    assert(lo <= result || result <= hi);

    return result;
}


/**
 *  \brief  Comparison function for \c size_t array q-sorting
 *
 *  \param  size1  LHS argument
 *  \param  size2  RHS argument
 *
 *  \retval -1 if \c size1 <  \c size2
 *  \retval  0 if \c size1 == \c size2
 *  \retval  1 if \c size1 >  \c size2
 */
static int cmp_size_t(const void *size1, const void *size2) {
    assert(NULL != size1);
    assert(NULL != size2);

    size_t size_1 = *(size_t *)size1;
    size_t size_2 = *(size_t *)size2;

    if (size_1 < size_2) return -1;
    if (size_1 > size_2) return  1;

    return 0;
}


/**
 *  \brief  Segment sequence constructor
 *
 *  \param  seg_seq    Segment sequence object (uninitialised)
 *  \param  data       Data to be segmented
 *  \param  data_size  Data size
 *  \param  offsets    Segmentation offsets
 *  \param  seg_cnt    Number of segments
 *
 *  \retval 0     on success
 *  \retval ENOMEM in case of allocation error
 */
static int segment_seq_create(segment_seq_t *seg_seq, char *data, size_t data_size,
                              const size_t offsets[], size_t seg_cnt) {
    assert(NULL != seg_seq);
    assert(NULL != data);
    assert(NULL != offsets);

    /* Create segment sequence */
    segment_t *seq = (segment_t *)calloc(seg_cnt, sizeof(segment_t));
    if (NULL == seq) {
        ERROR("Failed to allocate segment sequence");

        return ENOMEM;
    }

    size_t i = 0;

    for (; i < seg_cnt; ++i) {
        segment_t *seg = seq + i;

        seg->data  = data + offsets[i];
        seg->size  = (i < seg_cnt - 1 ? offsets[i + 1] : data_size) - offsets[i];
        seg->state = SEG_INITIALISED;

        DEBUX("%zu. segment %p (data %p, size %zu) was created",
              i + 1, seg, seg->data, seg->size);
    }

    seg_seq->impl = seq;
    seg_seq->cnt  = seg_cnt;

    return 0;
}


/**
 *  \brief  Segment sequence constructor (random segmentation)
 *
 *  \param  seg_seq    Segment sequence object (uninitialised)
 *  \param  data       Data to be segmented
 *  \param  data_size  Data size
 *
 *  \retval 0      on success
 *  \retval ENOMEM in case of allocation error
 */
static int segment_seq_create_rand(segment_seq_t *seg_seq, char *data, size_t data_size) {
    /* Choose random segment count */
    size_t seg_cnt = (size_t)rand_from(1, seg_max);

    DEBUG("%zu bytes of data shall be segmented into %zu chunks", data_size, seg_cnt);

    /* Create random segmentation offsets */
    size_t *offsets = (size_t *)malloc(seg_cnt * sizeof(size_t));
    if (NULL == offsets) {
        ERROR("Failed to allocate offsets");

        return ENOMEM;
    }

    /* First offset shall always be 0 */
    size_t i = 0;

    offsets[i] = 0;

    /* Set other segmentation offsets randomly */
    for (++i; i < seg_cnt; ++i)
        offsets[i] = rand_from(0, (int)data_size);

    /* Sort offsets */
    qsort(offsets + 1, seg_cnt - 1, sizeof(size_t), cmp_size_t);

#ifndef NDEBUG
    /* Sanity check */
    for (i = 0; i < seg_cnt; ++i) {
        assert(offsets[i] <= (i < seg_cnt - 1 ? offsets[i + 1] : data_size));

        DEBUX("%zu. segmentation offset: %zu", i + 1, offsets[i]);
    }
#endif  /* end of #ifndef NDEBUG */

    /* Create the sequence */
    int status = segment_seq_create(seg_seq, data, data_size, offsets, seg_cnt);

    free(offsets);

    return status;
}


/**
 *  \brief  Segment sequence destructor
 *
 *  \param  seg_seq  Segment sequence
 *
 *  \retval 0      on success
 *  \retval EINVAL in case one or more segments are still in use
 *
 */
static int segment_seq_destroy(segment_seq_t *seg_seq) {
    assert(NULL != seg_seq);

    int result = 0;

    size_t i = 0;

    for (; i < seg_seq->cnt; ++i) {
        segment_t *seg = seg_seq->impl + i;

        switch (seg->state) {
            case SEG_INITIALISED:
            case SEG_RELEASED:
                assert(0 == seg->use_cnt);

                break;

            case SEG_IN_USE:
                assert(0 < seg->use_cnt);

                ERROR("%zu. segment still in use by %u users "
                      "at point of sequence destruction",
                      i + 1, seg->use_cnt);

                result = EINVAL;

                break;
        }
    }

    free(seg_seq->impl);

    return result;
}


/**
 *  \brief  Get i-th segment of a sequence
 *
 *  \param  seg_seq  Segment sequence
 *  \param  index    Segment index
 *
 *  \return Segment at the index
 */
static segment_t *segment_seq_at(segment_seq_t *seg_seq, size_t index) {
    assert(NULL != seg_seq);
    assert(index < seg_seq->cnt);

    return seg_seq->impl + index;
}


/**
 *  \brief  Use segment data
 *
 *  Provides segment data, increments ref. counter
 *  and sets segment state accordingly.
 *
 *  \param  seg  Segment
 *
 *  \return Segment data
 */
static char *segment_use_data(segment_t *seg) {
    assert(NULL != seg);

    seg->state = SEG_IN_USE;
    ++seg->use_cnt;

    DEBUX("Segment %p is now used %u times", seg, seg->use_cnt);

    return seg->data;
}


/**
 *  \brief  Drop segment data
 *
 *  Decrements ref. counter and sets segment state accordingly.
 *
 *  \param  seg  Segment
 */
static void segment_drop_data(segment_t *seg) {
    assert(NULL != seg);
    assert(SEG_IN_USE == seg->state);
    assert(0 < seg->use_cnt);

    --seg->use_cnt;

    DEBUX("Segment %p is now used %u times", seg, seg->use_cnt);

    if (0 == seg->use_cnt) seg->state = SEG_RELEASED;
}


/**
 *  \brief  Cleanup routine for buffer/segment
 *
 *  \param  segment  Segment
 *  \param  data     Segment data
 *  \param  size     Segment size
 */
static void buffer_over_segment_cleanup(void *segment, char *data, size_t size) {
    segment_t *seg = (segment_t *)segment;

    DEBUX("Cleaning buffer over segment %p (data %p, size %zu)", seg, data, size);

    segment_drop_data(seg);
}


/**
 *  \brief  Process lexical items (alternative)
 *
 *  Lexical items are printed to std. output.
 *
 *  \param  items     List of items (parallel)
 *  \param  item_cnt  Count of items in the list
 */
static void process_lexical_items(const la_item_t *items, size_t item_cnt) {
    assert(NULL != items);

    INFO("%zu lexical item(s) suggested by lexical analyser:", item_cnt);

    if (!quiet) {
        if (compact) {
            fprintf(stdout, "%zu", item_cnt);
        }
        else {
            fprintf(stdout, "--- %zu%s lexical item%s printout ---\n", item_cnt,
                            (1 < item_cnt ? " parallel" : ""),
                            (1 < item_cnt ? "s" : ""));
        }
    }

    size_t i = 0;

    for (; i < item_cnt; ++i) {
        const la_item_t *const_item = items + i;

        int code = la_item_code(const_item);

        /* To obtain token, item copy is generally required */
        la_item_t item;

        la_item_copy(&item, const_item);

        size_t      token_len;
        const char *token      = la_item_token(&item, &token_len);
        size_t      token_off  = la_item_token_offset(&item);
        size_t      token_line = la_item_token_line(&item);
        size_t      token_col  = la_item_token_column(&item);

        INFO("%2zu: code: %d, pos: %zu:%zu (offset %zu), \"%.*s\" (%zu bytes)",
             i + 1, code, token_line, token_col, token_off, (int)token_len, token, token_len);

        if (!quiet) {
            if (compact) {
                fprintf(stdout, ";%d@%zu:%zu(%zu,%zu)\"%.*s\"",
                                code, token_line, token_col, token_off, token_len, (int)token_len, token);
            }
            else {
                fprintf(stdout, "%2zu) Item code: %d\n", i + 1, code);
                fprintf(stdout, "    Token: \"%.*s\" (length %zu)\n", (int)token_len, token, token_len);
                fprintf(stdout, "    Position: line %zu, column %zu, offset %zu\n", token_line, token_col, token_off);
            }
        }

        la_item_destroy(&item);
    }

    if (!quiet) {
        if (compact) {
            fprintf(stdout, "\n");
        }
        else {
            fprintf(stdout, "--- End of lexical item%s printout ---\n",
                            (1 < item_cnt ? "s" : ""));
        }
    }
}


/**
 *  \brief  Perform segmentation test
 *
 *  The input data segment sequence is treated as buffers of source
 *  which is re-segmented by the lexical analyser to lexical items.
 *
 *  Random part of segments are added at a time, either on demand
 *  or in advance.
 *
 *  The result is printed on std. output.
 *
 *  \param  lexa     Lexical analyser
 *  \param  seg_seq  Segment sequence (the source)
 *
 *  \retval 0      on success
 *  \retval EINVAL if the the lexical analyser failed
 */
static int segment_test(lexa_t *lexa, segment_seq_t *seg_seq) {
    size_t seg_added = 0;

    while (seg_added < segment_seq_cnt(seg_seq)) {
        /* Add random amount of segments */
        size_t to_add     = (size_t)rand_from(0, seg_add_max);
        size_t to_add_max = segment_seq_cnt(seg_seq) - seg_added;

        if (to_add > to_add_max)
            to_add = to_add_max;

        la_status_t la_status;

        while (to_add--) {
            segment_t *seg = segment_seq_at(seg_seq, seg_added);

            char   *data = segment_use_data(seg);
            size_t  size = segment_size(seg);
            int     last = seg_added + 1 == segment_seq_cnt(seg_seq);

            la_status = la_add_data(lexa, data, size,
                                    buffer_over_segment_cleanup, seg,
                                    last);
            if (LA_OK != la_status) {
                ERROR("Failed to feed lexical analyser with data: %d", la_status);

                segment_drop_data(seg);
                return EINVAL;
            }

            ++seg_added;
        }

        do {
            /* Get next lexical item */
            const la_item_t *items;
            size_t           item_cnt;

            la_status = la_get_items(lexa, &items, &item_cnt);

            switch (la_status) {
                case LA_OK: {
                    /* At least 1 item must be reported (EoF at end) */
                    if (!item_cnt) {
                        ERROR("At least 1 lexical item expected");

                        return EINVAL;
                    }

                    /* EoF reached */
                    if (LEXIG_EOF == la_item_code(items)) {
                        /* EoF may never be a matter of choice */
                        if (1 != item_cnt) {
                            ERROR("%zu items proposed, 1st being EoF", item_cnt);

                            return EINVAL;
                        }

                        DEBUG("Got end-of-file item, segmentation done");

                        if (!quiet) {
                            if (compact) {
                                fprintf(stdout, "Line OK\n");
                            }
                            else {
                                fprintf(stdout, "--- Line OK ---\n");
                            }
                        }

                        return 0;
                    }

                    process_lexical_items(items, item_cnt);

                    /* More than one item suggested, the choice may be wrong... */
                    if (1 < item_cnt) {
                        WARN("More than one lexical item were suggested "
                             "by lexical analyser, choosing the 1st one");
                    }

                    /* Read the 1st found item */
                    la_read_item(lexa, la_item_code(items + 0));

                    break;
                }

                case LA_INPUT_EXHAUSTED:
                    DEBUG("Input exhausted");

                    break;

                case LA_INPUT_INVALID:
                    WARN("Input can't be segmented further (no match)");

                    if (!quiet) {
                        if (compact) {
                            fprintf(stdout, "Line invalid\n");
                        }
                        else {
                            fprintf(stdout, "--- Line invalid ---\n");
                        }
                    }

                    return 0;

                case LA_ERROR:
                    ERROR("Failed to lexically segment source");

                    return EINVAL;
            }

        } while (LA_OK == la_status);
    }

    ERROR("All segments were already fed to lexical analyser, yet EoF wasn't reached");

    return EINVAL;
}


/**
 *  \brief  Segment input line
 *
 *  Depending on the lexical analyser definition, the input
 *  line shall be segmented (unless invalid).
 *
 *  The function doesn't actually do the segmentation itself.
 *  It's rather a wrapper that takes care of lexical analyser
 *  proper initialisation/finalisation and setting of the test
 *  environment (randomised segmentation of the input line
 *  into buffers and checking that the buffers are released
 *  after the line processing is finished).
 *
 *  \param  line      Input line
 *  \param  line_len  Length of the input line
 *
 *  \retval 0        on success
 *  \retval non-zero in case of error
 */
static int segment_line(char *line, size_t line_len) {
    assert(NULL != line);

    int status;

    /* Create random segmentation sequence of the input line */
    segment_seq_t seg_seq;

    status = segment_seq_create_rand(&seg_seq, line, line_len);
    if (status) {
        ERROR("Failed to create random segment sequence: %d", status);

        return status;
    }

    /* Create lexical analyser */
    lexa_t lexa;

    la_create(&lexa, &test_fsa, LEXICNT);

    /* Test lexical analyser */
    status = segment_test(&lexa, &seg_seq);

    /* Destroy lexical analyser */
    la_destroy(&lexa);

    /* Destroy segment sequence */
    segment_seq_destroy(&seg_seq);

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
    LOG("    -q                  suppress lexical items printout");
    LOG("    -c                  compact lexical items printout (suitable for automatic testing)");
    LOG("    -l <log level>      set log level, default: %d (%s)", DEFAULT_LOG_LEVEL, log_lvl2str(DEFAULT_LOG_LEVEL));
    LOG("    -s <seg. count>     set maximal segment count, default: %d", DEFAULT_MAX_SEG_CNT);
    LOG("    -a <seg. count>     set maximal count of segmemnts added at a time, default: %d", DEFAULT_MAX_SEG_ADD);
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

    while (-1 != (opt = getopt(argc, argv, "hqcl:s:a:S:"))) {
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

            case 'l':
                log_level = atoi(optarg);

                break;

            case 's':
                seg_max = atoi(optarg);

                break;

            case 'a':
                seg_add_max = atoi(optarg);

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

    if (seg_max < 0) {
        FATAL("Maximal segment count is invalid: %d", seg_max);

        usage(argv[0]);
        exit(1);
    }

    if (seg_add_max < 0) {
        FATAL("Maximal added segment count is invalid: %d", seg_add_max);

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
        TEST_CASE(line, segment_line, line, line_len);
    }

    free(line);
    line = NULL;

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
