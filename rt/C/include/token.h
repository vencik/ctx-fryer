#ifndef token_h
#define token_h

/**
 *  \brief   Lexical token
 *
 *  Token interface provided by this module allows get-only
 *  access to underlaying lexical items reader.
 *  Note that the tokens are only valid as long as the parse
 *  tree is valid.
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2013/03/28
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

#include <stddef.h>


typedef void token_t;  /**< Lexical token opaque handle */


/** Lexical token conversion status */
typedef enum {
    TOKEN_CONV_OK = 0,          /**< Successful conversion    */
    TOKEN_CONV_UNSUPPORTED,     /**< Connection not supported */
    TOKEN_CONV_EMPTY,           /**< Token is empty           */
    TOKEN_CONV_PREMATURE_END,   /**< Premature end of string  */
    TOKEN_CONV_INVALID_CHAR,    /**< Unacceptable character   */
    TOKEN_CONV_INVALID_FORMAT,  /**< Target format violation  */
    TOKEN_CONV_ERROR            /**< Other error              */
} token_conv_status_t;  /* end of typedef enum */


/**
 *  \brief  Get lexical token data buffer
 *
 *  The function provides read-only access to token data buffer.
 *  Note that the function DOESN'T return a C-string
 *  (i.e. the data isn't null-terminated).
 *  See \ref token_string for that.
 *  Also see \ref token_length.
 *
 *  Note that the returned pointer is only valid as long as the parse tree is.
 *  If you need a copy, either do it yourself or use the \ref token_string
 *  function.
 *
 *  Also note that the function may eventually fail (in which case it returns
 *  \c NULL).
 *  The reason is that the token data defragmentation might be necessary,
 *  in which case memory allocation may ocur.
 *  However, necessity for allocation should be very rare in practice.
 *  The defragmented token is cached for repeated direct usage in future
 *  of course, so it occurs at most once.
 *
 *  \param  token  Token
 *
 *  \return Token data or \c NULL in case of memory error
 */
const char *token_data(token_t *token);


/**
 *  \brief  Get lexical token C-string copy
 *
 *  The function provides copy of the token data terminated by the null
 *  character (i.e. so-called C-string).
 *  The returned data becomes responsability of the caller and should
 *  be released by calling \c free when no longer used.
 *
 *  Note that the whole token is always copied, no matter whether it contains
 *  any null characters on its own.
 *  That means that you can use it for copying binary data tokens, either.
 *  Of course, if you'll try to use such data as C-strings any further,
 *  you must anticipate that C-string manipulators will trim the data
 *  (which probably isn't what you want).
 *
 *  Also note that if you need the token just for output or other simple
 *  things, you should always consider using \ref token_data instead since
 *  the other function saves you the memory allocation and string copying.
 *  All decent implementations of \c *printf functions should accept
 *  \c "%.*s" and \c "%.<pos>$s" format strings for raw char. buffers
 *  with extra length argument.
 *  See \c man \c fprintf.
 *
 *  \param  token  Token
 *
 *  \return Dynamic-memory copy of the token data as C-string
 *          or \c NULL in case of memory error
 */
char *token_string(token_t *token);


/**
 *  \brief  Get lexical token length
 *
 *  \param  token  Token
 *
 *  \return Token length
 */
size_t token_length(token_t *token);


/**
 *  \brief  Get lexical token offset
 *
 *  \param  token  Token
 *
 *  \return Token offset in source (relative to the beginning)
 */
size_t token_offset(token_t *token);


/**
 *  \brief  Get lexical token line
 *
 *  \param  token  Token
 *
 *  \return Token line in line-based source
 */
size_t token_line(token_t *token);


/**
 *  \brief  Get lexical token column
 *
 *  \param  token  Token
 *
 *  \return Token column in line-based source
 */
size_t token_column(token_t *token);


/**
 *  \brief  Convert token to integer
 *
 *  The function implements conversion of the token
 *  to a signed integer in arbitrary radix.
 *  The expected token syntax is
 *  \c /^(-|\+)?[0-9a-zA-Z]+$/
 *  where the alphabetic characters represent digits of
 *  10 and following in case insensitive manner as usual.
 *  That BTW means that the max. radix base is 26 (min. being 2).
 *
 *  \param[in]   token    Token
 *  \param[in]   base     Radix base
 *  \param[out]  integer  Result of the conversion
 *
 *  \retval TOKEN_CONV_OK             on success
 *  \retval TOKEN_CONV_UNSUPPORTED    if radix base is not supported
 *  \retval TOKEN_CONV_EMPTY          if the token is empty
 *  \retval TOKEN_CONV_PREMATURE_END  if the token only contains the sign
 *  \retval TOKEN_CONV_INVALID_CHAR   if the token contains an invalid character
 *  \retval TOKEN_CONV_INVALID_FORMAT if the token contains a digit
 *                                    that is out of the radix base
 *  \retval TOKEN_CONV_ERROR          in case of generic failure
 */
token_conv_status_t token_to_integer(
    token_t   *token,
    long long  base,
    long long *integer);


/**
 *  \brief  Convert token to floating-point number
 *
 *  The function implements conversion of the token
 *  to a signed decimal floating-point number.
 *  The expected token syntax is
 *  \c /^(-|\+)?([0-9]+|[0-9]*\.[0-9]+|[0-9]+\.[0-9]*)((e|E)(-|\+)?[0-9]+)?$/
 *  (i.e. the standard syntax allowing scientific notation).
 *  At least one digit of the mantisa must be present (no matter
 *  on the floating point position).
 *  The exponential (scale) part has the normal semantics of
 *  mantisa * 10^scale.
 *
 *  Examples: 3.1415926536 +2.14 -123.456e-3 .0 2. (but not just . or -. etc).
 *
 *  \param[in]   token  Token
 *  \param[out]  f1oat  Result of the conversion
 *
 *  \retval TOKEN_CONV_OK             on success
 *  \retval TOKEN_CONV_EMPTY          if the token is empty
 *  \retval TOKEN_CONV_PREMATURE_END  if the token only contains the sign
 *  \retval TOKEN_CONV_INVALID_CHAR   if the token contains an invalid character
 *  \retval TOKEN_CONV_INVALID_FORMAT if the floating-point number format
 *                                    was violated
 *  \retval TOKEN_CONV_ERROR          in case of generic failure
 */
token_conv_status_t token_to_float(token_t *token, long double *f1oat);

#endif  /* end of #ifndef token_h */
