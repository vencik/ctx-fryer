/**
 *  \brief   Built-in attributes
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

#include "builtin_attr.h"
#include "attribute.h"
#include "ptree.h"
#include "token.h"

#include <assert.h>


/*
 * Attribute built-in functions definitions
 */

attr_eval_status_t builtin__get_token(attr_handle_t *attr, token_t **token) {
    assert(NULL != attr);
    assert(NULL != token);

    ptree_node_t *ptnode = attr_ptree_node(attr);

    assert(NULL != ptnode);

    la_item_t *lex_item = ptree_tnode_item(ptnode);

    if (NULL == lex_item) return ATTR_EVAL_ERROR;

    *token = lex_item;  /* la_item_t implements token */

    return ATTR_EVAL_OK;
}
