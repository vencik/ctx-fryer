#include "test.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


char * const test_log_levels[] = {
    /* LOG_LVL_ALL   */  "**",
    /* LOG_LVL_FATAL */  "!!",
    /* LOG_LVL_ERROR */  "EE",
    /* LOG_LVL_WARN  */  "WW",
    /* LOG_LVL_INFO  */  "II",
    /* LOG_LVL_DEBUG */  "DD",
    /* LOG_LVL_DEBUX */  "XD",
};

const char *test_log_level_unknown = "??";


ssize_t snprintf_size_t(char *dest, size_t avail, const char *format, void *item) {
    assert(NULL != dest);
    assert(NULL != item);

    return (ssize_t)snprintf(dest, avail, format, *(size_t *)item);
}


char *array2str_impl(void *array, size_t cnt, size_t size,
                     const char *format, const char *sep,
                     ssize_t (*item_format_fn)(char * , size_t , const char * , void * )) {
    assert(NULL != array);
    assert(NULL != sep);

    #define SIZE_INC 1024

    size_t str_size = SIZE_INC;
    char *str = (char *)malloc(str_size * sizeof(char));

    if (NULL == str) return NULL;

    /* Empty array */
    if (0 == cnt) {
        str[0] = '\0';
        return str;
    }

    size_t pos = 0;
    size_t i   = 0;

    int item_is_next = 1;

    do {
        size_t  space_left = str_size - pos;
        ssize_t diff;

        /* Stringify an item */
        if (item_is_next) {
            diff = item_format_fn(str + pos, space_left, format, array + i * size);
        }

        /* Stringify separator */
        else {
            diff = (ssize_t)snprintf(str + pos, space_left, "%s", sep);
        }

        /* Error */
        if (diff < 0) {
            free(str);
            str = NULL;
        }

        /* Not enough space, reallocation required */
        else if (diff >= space_left) {
            str_size += SIZE_INC;

            char *new_str = (char *)realloc(str, str_size);
            if (NULL == new_str) free(str);

            str = new_str;
        }

        /* Stringified OK */
        else {
            /* Another item was stringified */
            if (item_is_next) {
                ++i;

                /* Done */
                if (i == cnt) break;

                item_is_next = 0;
            }

            /* Separator was stringified */
            else item_is_next = 1;

            pos += diff;
        }

    } while (NULL != str);

    #undef SIZE_INC

    return str;
}
