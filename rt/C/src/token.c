/**
 *  \brief   Lexical token implementation
 *
 *  IMPLEMENTATION NOTES:
 *  * Lexical token is implemented directly by lexical item
 *  * Since only get interface is provided to the user,
 *    no destructor is required (lexical items are destroyed
 *    automatically upon parse tree destruction)
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2013/03/31
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

#include "token.h"
#include "lexical_analyser.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/*
 * Static functions prototypes
 */

inline static token_conv_status_t token_to_float_impl(
    const char  *token,
    size_t       length,
    long double *f1oat);


/*
 * Token interface implementation
 */

const char *token_data(token_t *token) {
    assert(NULL != token);

    size_t length;

    return la_item_token((la_item_t *)token, &length);
}


char *token_string(token_t *token) {
    assert(NULL != token);

    size_t length;

    const char *data = la_item_token((la_item_t *)token, &length);

    if (NULL == data) return NULL;

    char *string = (char *)malloc((length + 1) * sizeof(char));

    if (NULL == string) return NULL;

    memcpy(string, data, length);

    string[length] = '\0';

    return string;
}


size_t token_length(token_t *token) {
    assert(NULL != token);

    return la_item_token_length((la_item_t *)token);
}


size_t token_offset(token_t *token) {
    assert(NULL != token);

    return la_item_token_offset((la_item_t *)token);
}


size_t token_line(token_t *token) {
    assert(NULL != token);

    return la_item_token_line((la_item_t *)token);
}


size_t token_column(token_t *token) {
    assert(NULL != token);

    return la_item_token_column((la_item_t *)token);
}


token_conv_status_t token_to_integer(
    token_t   *token,
    long long  base,
    long long *integer)
{
    assert(NULL != token);
    assert(NULL != integer);

    if (!(2 <= base && base <= 26))
        return TOKEN_CONV_UNSUPPORTED;

    size_t length, i = 0;

    int negative = 0;

    const char *data = la_item_token((la_item_t *)token, &length);

    /* Token defragmentation error (memory error) */
    /* TBD: Perhaps this status should be propagated more distinctly */
    if (NULL == data) return TOKEN_CONV_ERROR;

    /* Empty token is not valid integer */
    if (0 == length) return TOKEN_CONV_EMPTY;

    /* Sign */
    if ('-' == data[i]) {
        negative = 1;

        ++i;
    }
    else if ('+' == data[i])
        ++i;

    /* At least one digit is mandatory */
    if (!(i < length)) return TOKEN_CONV_PREMATURE_END;

    /* Collect digits */
    *integer = 0;

    do {
        long long inc;

        char digit = data[i];

        /* Decimal digits */
        if ('0' <= digit && digit <= '9')
            inc = digit - '0';

        /* Lowercase alphabetic digits */
        else if ('a' <= digit && digit <= 'z')
            inc = digit - 'a' + 10;

        /* Uppercase alphabetic digits */
        else if ('A' <= digit && digit <= 'Z')
            inc = digit - 'A' + 10;

        /* Unacceptable character */
        else return TOKEN_CONV_INVALID_CHAR;

        /* Unacceptable digit */
        if (!(inc < base)) return TOKEN_CONV_INVALID_FORMAT;

        *integer *= base;
        *integer += inc;

    } while (++i < length);

    /* Negation */
    if (negative)
        *integer = -*integer;

    return TOKEN_CONV_OK;  /* single point of success */
}


token_conv_status_t token_to_float(token_t *token, long double *f1oat) {
    assert(NULL != token);
    assert(NULL != f1oat);

    int negative = 0;

    size_t length, i = 0;

    const char *data = la_item_token((la_item_t *)token, &length);

    /* Token defragmentation error (memory error) */
    /* TBD: Perhaps this status should be propagated more distinctly */
    if (NULL == data) return TOKEN_CONV_ERROR;

    /* Empty token is not valid float */
    if (0 == length) return TOKEN_CONV_EMPTY;

    /* Sign (and sign alone is unacceptable) */
    if ('-' == data[i]) {
        negative = 1;

        ++i;

        if (1 == length) return TOKEN_CONV_PREMATURE_END;
    }
    else if ('+' == data[i]) {
        ++i;

        if (1 == length) return TOKEN_CONV_PREMATURE_END;
    }

    /* Sign alone is not acceptable */

    *f1oat = 0.0;  /* initialise now so we can negate it even on conv. error */

    /* The actual parsing (best to be inlined) */
    token_conv_status_t status = token_to_float_impl(data + i, length - i, f1oat);

    /* Negation */
    if (negative)
        *f1oat = -*f1oat;

    return status;
}


/*
 * Static functions definitions
 */

/**
 *  \brief  Token to floating point number conversion implementation
 *
 *  The function implements the conversion, except for parsing (and application)
 *  of the sign.
 *  It is extracted from the \ref token_to_float function merely to allow early
 *  jump to the sign application (avoiding usage of goto).
 *
 *  \param[in]   token   Token data (not including sign)
 *  \param[in]   length  Token data length (must be non-zero)
 *  \param[out]  f1oat   The result (already initialised to 0.0)
 *
 *  \retval TOKEN_CONV_OK             on successful conversion
 *  \retval TOKEN_CONV_PREMATURE_END  if the token specifies only valid prefix
 *  \retval TOKEN_CONV_INVALID_CHAR   if the token is unacceptable number spec.
 *  \retval TOKEN_CONV_INVALID_FORMAT if the floating-point number format
 *                                    was violated
 */
inline static token_conv_status_t token_to_float_impl(
    const char  *token,
    size_t       length,
    long double *f1oat)
{
    assert(NULL != token);
    assert(NULL != f1oat);

    size_t digit_cnt = 0;
    size_t i         = 0;

    assert(i < length);

    /* Collect integral part digits */
    char ch;

    for (;;) {
        ch = token[i];

        /* Floating point */
        if ('.' == ch) break;

        /* Early exponent */
        if ('e' == ch || 'E' == ch) break;

        /* Unaceptable character */
        if (!('0' <= ch && ch <= '9'))
            return TOKEN_CONV_INVALID_CHAR;

        *f1oat *= 10;
        *f1oat += ch - '0';

        ++digit_cnt;

        /* Done (no fraction part nor exponent) */
        if (!(++i < length)) return TOKEN_CONV_OK;
    }

    /* Sanity check */
    assert(i < length && ('.' == ch || 'e' == ch || 'E' == ch));

    /* Collect fraction part digits */
    if ('.' == ch) {
        long double q = 10.0;

        for (;;) {
            /* Done (no exponent) */
            if (!(++i < length))
                return
                    0 == digit_cnt
                    ? TOKEN_CONV_INVALID_FORMAT
                    : TOKEN_CONV_OK;

            ch = token[i];

            /* Exponent */
            if ('e' == ch || 'E' == ch) break;

            /* Unaceptable character */
            if (!('0' <= ch && ch <= '9'))
                return TOKEN_CONV_INVALID_CHAR;

            *f1oat += (long double)(ch - '0') / q;

            ++digit_cnt;

            q *= 10;
        }
    }

    /* At least one digit is mandatory */
    if (0 == digit_cnt) return TOKEN_CONV_INVALID_FORMAT;

    /* Sanity check */
    assert(i < length && ('e' == ch || 'E' == ch));

    ++i;  /* consume scale mark */

    /* Empty scale specification is unacceptable */
    if (!(i < length)) return TOKEN_CONV_PREMATURE_END;

    /* Scale sign */
    int negative_scale = 0;

    if ('-' == token[i]) {
        negative_scale = 1;

        ++i;
    }
    else if ('+' == token[i])
        ++i;

    /* Empty scale specification is (still) unacceptable */
    if (!(i < length)) return TOKEN_CONV_PREMATURE_END;

    /* Collect scale */
    size_t scale = 0;

    do {
        ch = token[i];

        /* Unaceptable character */
        if (!('0' <= ch && ch <= '9'))
            return TOKEN_CONV_INVALID_CHAR;

        scale *= 10;
        scale += ch - '0';

    } while (++i < length);

    /* Apply scale */
    long double q = 1.0;

    for (i = 0; i < scale; ++i)
        q *= 10;

    if (negative_scale)
        *f1oat /= q;
    else
        *f1oat *= q;

    return TOKEN_CONV_OK;
}
