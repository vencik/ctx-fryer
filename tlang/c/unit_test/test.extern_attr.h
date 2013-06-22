#ifndef test__extern_attr_h
#define test__extern_attr_h

/**
 *  \brief   External attributes functions declarations
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2013/04/14
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

#include "attribute.h"


/**
 *  \brief  Summation
 *
 *  \param[out]  result  Result
 *  \param[in]   l_arg   Left argument
 *  \param[in]   r_arg   Right argument
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR on memory error (result is dynamic)
 */
attr_eval_status_t sum(void **result, void *l_arg, void *r_arg);


/**
 *  \brief  Multiplication
 *
 *  \param[out]  result  Result
 *  \param[in]   l_arg   Left argument
 *  \param[in]   r_arg   Right argument
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR on memory error (result is dynamic)
 */
attr_eval_status_t mul(void **result, void *l_arg, void *r_arg);


/**
 *  \brief  Token-to-double convertor
 *
 *  \param[out]  f1oat  Floating point number
 *  \param[in]   token  Lexical token
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR on memory error (result is dynamic)
 */
attr_eval_status_t token2num(void **f1oat, void *token);


/**
 *  \brief  Value destructor
 *
 *  \param[in]  val  Value
 */
void destroy_value(void *val);

#endif  /* end of #ifndef test__extern_attr_h */
