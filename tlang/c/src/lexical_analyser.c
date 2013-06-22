/**
 *  \brief  Lexical analyser: definitions
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/06/22
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

#include "lexical_analyser.h"
#include "buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/*
 * Static routines forward declarations
 */

static la_status_t la_get_item__impl(lexa_t *la);

inline static void la_get_item__add_state_accepts(lexa_t *la);

inline static const fsa_state_t *la_get_item__follow_branch(lexa_t *la);

static void la_free_token(void *null, char *data, size_t length);


/*
 * Module interface implementation
 */

lexa_t *la_create(lexa_t *la, const fsa_t *fsa, size_t items_total) {
    assert(NULL != la);
    assert(NULL != fsa);

    size_t    *pos   = (size_t *)malloc(items_total * sizeof(size_t));
    la_item_t *items = (la_item_t *)malloc(items_total * sizeof(la_item_t));

    /* Memory fault */
    if (NULL == items || NULL == pos) {
        free(pos);
        free(items);

        return NULL;
    }

    /* Initialisation */
    memset(la, 0, sizeof(*la));

    size_t i = 0;

    for (; i < items_total; ++i) {
        pos[i] = items_total;
    }

    la->fsa                = fsa;
    la->state.fsa_state    = fsa_root(fsa);
    la->state.item_list    = items;
    la->state.item_pos     = pos;
    la->state.token_line   = 1;
    la->state.token_column = 1;
    la->state.line         = 1;
    la->state.column       = 1;
    la->items_total        = items_total;
    la->status             = LA_OK;

    return la;
}


void la_destroy(lexa_t *la) {
    assert(NULL != la);

    /* Unreference remaining source buffers */
    while (NULL != la->state.token_buff) {
        buffer_t *next = buffer_get_next(la->state.token_buff);

        buffer_unref(la->state.token_buff);

        if (la->state.token_buff == la->buff_last) break;

        la->state.token_buff = next;
    }

    free(la->state.item_pos);
    free(la->state.item_list);
}


la_status_t la_add_data(lexa_t *la, char *data, size_t size, buffer_cleanup_fn *cleanup_fn, void *user_obj, int is_last) {
    /* Create buffer */
    buffer_t *buffer = buffer_create(data, size, cleanup_fn, user_obj, is_last);

    if (NULL == buffer)
        return la->status = LA_ERROR;

    /* Add buffer */
    la_add_buffer(la, buffer);

    buffer_unref(buffer);

    return LA_OK;
}


void la_add_buffer(lexa_t *la, buffer_t *buffer) {
    assert(NULL != la);

    buffer_ref(buffer);

    if (la->buff_last) {
        buffer_set_next(la->buff_last, buffer);
        buffer_set_prev(buffer, la->buff_last);
    }

    la->buff_last = buffer;

    if (NULL == la->buffer)
        la->buffer = buffer;

    if (NULL == la->state.token_buff)
        la->state.token_buff = buffer;
}


la_status_t la_get_items(lexa_t *la, const la_item_t **items, size_t *item_cnt) {
    assert(NULL != la);

    la_status_t status = la_get_item__impl(la);

    if (NULL != items)
        *items = la->state.item_list;

    if (NULL != item_cnt)
        *item_cnt = la->state.item_cnt;

    return status;
}


const la_item_t *la_get_item(lexa_t *la, int item_code) {
    assert(NULL != la);

    la_get_item__impl(la);

    size_t item_idx = la->state.item_pos[item_code];

    la_item_t *item = NULL;

    if (item_idx < la->items_total) {
        item = la->state.item_list + item_idx;

        assert(item->code == item_code);
    }

    return item;
}


la_status_t la_read_item(lexa_t *la, int item_code) {
    assert(NULL != la);
    assert(0 <= item_code && item_code <= la->items_total);

    /* Get item position */
    size_t item_idx = la->state.item_pos[item_code];

    if (!(item_idx < la->state.item_cnt)) return LA_ERROR;

    /* Reading only makes sense after complete segmentation step */
    assert(NULL == la->state.fsa_state);

    /* Reading from end of file doesn't make sense, either */
    assert(LEXIG_EOF != la->state.item_list[0].code);

    /* Set next token source offset */
    la->state.token_offset += la->state.item_list[item_idx].length;

    /* Set source file position */
    la->state.line = la->state.token_line =
        la->state.item_list[item_idx].next_line;
    la->state.column = la->state.token_column =
        la->state.item_list[item_idx].next_column;

    /* Compensate for potential lexical item overrun */
    la->state.token_length -= la->state.item_list[item_idx].length;

    if (NULL == la->buffer) {
        la->buffer = la->buff_last;
        la->state.buffer_offset = buffer_size(la->buffer);
    }

    while (0 < la->state.token_length) {
        size_t buffer_diff = la->state.token_length;

        /* Go one buffer backwards if token spans over multiple buffers */
        if (buffer_diff > la->state.buffer_offset) {
            buffer_diff = la->state.buffer_offset;

            la->buffer = la->buffer->prev;
            la->state.buffer_offset = buffer_size(la->buffer);
        }
        else {
            la->state.buffer_offset -= buffer_diff;
        }

        la->state.token_length -= buffer_diff;
    }

    la->state.token_length = 0;

    /* Unreference all used-up buffers */
    while (la->state.token_buff != la->buffer) {
        assert(NULL != la->state.token_buff);

        buffer_t *next = buffer_get_next(la->state.token_buff);

        buffer_unref(la->state.token_buff);

        la->state.token_buff = next;
    }

    la->state.token_buff_off = la->state.buffer_offset;

    /* Reset listed items and their positions (list is non-empty) */
    do {
        --la->state.item_cnt;

        la_item_t *item = &la->state.item_list[la->state.item_cnt];

        la->state.item_pos[item->code] = la->items_total;

#ifndef NDEBUG
        memset(item, 0, sizeof(*item));
#endif  /* end of #ifndef NDEBUG */

    } while (0 < la->state.item_cnt);

    /* Make the analyser ready for further segmentation */
    la->state.fsa_state = fsa_root(la->fsa);

    /* Reset status */
    return la->status = LA_OK;
}


const char *la_item_token(la_item_t *item, size_t *length) {
    assert(NULL != item);
    assert(NULL != length);

    *length = item->length;

    buffer_t *buffer = item->buff_1st;
    size_t    offset = item->buff_off;
    size_t    size   = buffer_size(buffer) - offset;

    /* Token isn't fragmented */
    if (*length <= size)
        return buffer_data(buffer) + offset;

    /* Create new buffer for token copy */
    char *data = (char *)malloc(*length * sizeof(char));
    if (NULL == data)
        return NULL;

    buffer_t *new_buffer = buffer_create(data, *length, &la_free_token, NULL, 1);
    if (NULL == new_buffer) {
        free(data);
        return NULL;
    }

    /* Concatenate buffers */
    size_t to_go = *length;
    for (;;) {
        memcpy(data, buffer_data(buffer) + offset, size);

        data  += size;
        to_go -= size;

        item->buff_1st = buffer_get_next(buffer);

        buffer_unref(buffer);

        if (0 == to_go) break;

        buffer = item->buff_1st;

        assert(NULL != buffer);

        size = buffer_size(buffer);
        if (to_go < size) size = to_go;

        offset = 0;
    }

    /* Set new buffer */
    item->buff_1st = new_buffer;
    item->buff_off = 0;

    return buffer_data(new_buffer);
}


void la_item_copy(la_item_t *dest, const la_item_t *src) {
    assert(NULL != dest);
    assert(NULL != src);

    memcpy(dest, src, sizeof(la_item_t));

    /* Reference buffer(s) for the destination */
    buffer_t *buffer = dest->buff_1st;
    size_t    offset = dest->buff_off;
    size_t    length = dest->length;

    while (0 < length) {
        buffer_ref(buffer);

        size_t size = buffer_size(buffer) - offset;

        length -= size < length ? size : length;

        buffer = buffer_get_next(buffer);
        offset = 0;
    }
}


void la_item_destroy(la_item_t *item) {
    assert(NULL != item);

    /* Unreference buffer(s) */
    size_t offset = item->buff_off;
    size_t length = item->length;

    while (0 < length) {
        buffer_t *buffer = item->buff_1st;

        item->buff_1st = buffer_get_next(buffer);

        size_t size = buffer_size(buffer) - offset;

        length -= size < length ? size : length;

        buffer_unref(buffer);

        offset = 0;
    }

    /* The item is invalidated */
    item->code = LEXIG_INVALID;
}


/*
 * Static routines definitions
 */

/**
 *  \brief  \ref la_get_item implementation
 *
 *  \ref la_get_item routine is in fact just a wrapper that provides
 *  the user with result of this function.
 *
 *  Note that the function is only ment as part of the \ref la_get_item
 *  function and is torn from it just to increase readablility.
 *  It changes state of the lexical analyser.
 *  Do not use the function for any other purposes.
 *
 *  \param  la  Lexical analyser
 *
 *  \return Lexical analyser status
 */
static la_status_t la_get_item__impl(lexa_t *la) {
    assert(NULL != la);

    /* Input is no more exhausted */
    if (LA_INPUT_EXHAUSTED == la->status && la->buffer)
        la->status = LA_OK;

    /* Can't do anything right now */
    else if (LA_OK != la->status)
        return la->status;

    /* Item(s) already available */
    else if (la->state.item_cnt)
        return LA_OK;

    assert(LA_OK == la->status);

    /* Input buffer unavailable */
    if (!la->buffer)
        return la->status = LA_INPUT_EXHAUSTED;

    /* Parsing is necessary */
    for (;; la->state.seg_int = 0) {
        /* Add items accepted by the current FSA state */
        if (!la->state.seg_int)
            la_get_item__add_state_accepts(la);

        /* No branches */
        if (0 == la->state.fsa_state->branch_cnt) {
            /* Dead end is a logical error */
            assert(la->state.item_cnt > 0);

            la->state.fsa_state = NULL;

            assert(LA_OK == la->status);

            return LA_OK;
        }

        /* Buffer ends */
        while (buffer_size(la->buffer) <= la->state.buffer_offset) {
            if (buffer_is_last(la->buffer)) {
                /* At least one item was parsed */
                if (la->state.item_cnt)
                    la->state.fsa_state = NULL;

                /* EoF (single item) */
                else if (0 == la->state.token_length) {
                    la->state.item_list[0].code     = LEXIG_EOF;
                    la->state.item_list[0].offset   = la->state.token_offset;
                    la->state.item_list[0].length   = 0;
                    la->state.item_list[0].line     = la->state.line;
                    la->state.item_list[0].column   = la->state.column;
                    la->state.item_list[0].buff_1st = NULL;
                    la->state.item_list[0].buff_off = 0;

                    la->state.item_cnt  = 1;
                    la->state.fsa_state = NULL;

                    assert(LA_OK == la->status);
                }

                /* Trailing characters */
                else
                    la->status = LA_INPUT_INVALID;

                return la->status;
            }

            buffer_t *next = buffer_get_next(la->buffer);
            la->state.buffer_offset = 0;
            la->buffer = next;

            /* Next buffer not available yet */
            if (!next) {
                la->state.seg_int = 1;

                return la->status = LA_INPUT_EXHAUSTED;
            }
        }

        /* Branch by the next character in the input */
        if (NULL == la_get_item__follow_branch(la)) {
            /* No items mean invalid input */
            la->status = la->state.item_cnt
                       ? LA_OK : LA_INPUT_INVALID;

            return la->status;
        }
    }

    assert("Logical error: Unreachable code reached" == NULL);
}


/**
 *  \brief  Add lexical items accepted by current FSA state to the item list
 *
 *  Note that the function is only ment as part of the \ref la_get_item
 *  function and is torn from it just to increase readablility.
 *  It changes state of the lexical analyser.
 *  Do not use the function for any other purposes.
 *
 *  \param  la  Lexical analyser
 */
inline static void la_get_item__add_state_accepts(lexa_t *la) {
    assert(NULL != la);

    size_t i = 0;

    for (; i < la->state.fsa_state->accept_cnt; ++i) {
        int    code = la->state.fsa_state->accepts[i];
        size_t pos  = la->state.item_pos[code];

        /* New item */
        if (pos == la->items_total) {
            pos = la->state.item_cnt++;

            la->state.item_pos[code] = pos;

            la->state.item_list[pos].code     = code;
            la->state.item_list[pos].offset   = la->state.token_offset;
            la->state.item_list[pos].line     = la->state.token_line;
            la->state.item_list[pos].column   = la->state.token_column;
            la->state.item_list[pos].buff_1st = la->state.token_buff;
            la->state.item_list[pos].buff_off = la->state.token_buff_off;
        }

        /* Redefinition */
        else {
            assert(la->state.item_list[pos].code == code);
        }

        la->state.item_list[pos].length      = la->state.token_length;
        la->state.item_list[pos].next_line   = la->state.line;
        la->state.item_list[pos].next_column = la->state.column;
    }
}


/**
 *  \brief  Take branch matching character from current FSA state
 *
 *  If there's no matching branch, the function sets current state
 *  to \c NULL.
 *
 *  Note that the function is only ment as part of the \ref la_get_item
 *  function and is torn from it just to increase readablility.
 *  It changes state of the lexical analyser.
 *  Do not use the function for any other purposes.
 *
 *  \param  la  Lexical analyser
 *
 *  \retval  Current FSA state (for branch success checks)
 */
inline static const fsa_state_t *la_get_item__follow_branch(lexa_t *la) {
    assert(NULL != la);
    assert(NULL != la->buffer);
    assert(la->state.buffer_offset < buffer_size(la->buffer));

    char ch = buffer_data(la->buffer)[la->state.buffer_offset];

    size_t i = 0;

    for (i = 0; i < la->state.fsa_state->branch_cnt; ++i) {
        const fsa_branch_t *branch = la->state.fsa_state->branches[i];

        size_t j = 0;

        for (; j < branch->interval_cnt; ++j) {
            char lo = branch->intervals[j][0];
            char hi = branch->intervals[j][1];

            /* Matching branch found */
            if (lo <= ch && ch <= hi) {
                ++la->state.buffer_offset;
                ++la->state.token_length;

                if ('\n' == ch) {
                    ++la->state.line;
                    la->state.column = 1;
                }
                else
                    ++la->state.column;

                la->state.fsa_state = branch->target;

                return la->state.fsa_state;
            }
        }
    }

    return la->state.fsa_state = NULL;
}


/**
 *  \brief  Free lexical token
 *
 *  Used ac cleanup callback for joined tokens.
 *
 *  \param  null    \c NULL (argument not used)
 *  \param  data    Token
 *  \param  length  Token length (irrelevant)
 */
static void la_free_token(void *null, char *data, size_t length) {
    free(data);
}
