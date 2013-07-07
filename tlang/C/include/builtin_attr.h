#ifndef builtin_attr_h
#define builtin_attr_h

/**
 *  \brief   Built-in attributes
 *
 *  Built-in attributes, their evaluators & destructors.
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

#include "attribute.h"
#include "token.h"


/**
 *  \brief  Lexical token getter
 *
 *  \param  attr   Attribute
 *  \param  token  Target
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR if the associated parse tree node
 *                          doesn't represent a terminal on the input
 */
attr_eval_status_t builtin__get_token(attr_handle_t *attr, token_t **token);

#endif  /* end of #ifndef builtin_attr_h */
