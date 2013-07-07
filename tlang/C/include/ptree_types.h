#ifndef CTXFryer__ptree_types_h
#define CTXFryer__ptree_types_h

/**
 *  \brief  Parse tree types
 *
 *  Type definitions of parse tree module.
 *  Extracted from \ref ptree.h due necessity of allowing
 *  back-references (i.e. in the attribute interface,
 *  see \ref attribute.h).
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2013/03/29
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


/** Parse tree node type */
typedef enum {
    PTREE_TNODE,  /**< Terminal node     */
    PTREE_NTNODE  /**< Non-terminal node */
} ptree_node_type_t;  /* end of typedef enum */


typedef struct ptree_node         ptree_node_t;          /**< Parse tree node                       */
typedef struct ptree_node_payld   ptree_node_payld_t;    /**< Parse tree node payload               */
typedef union  ptree_snode_payld  ptree_snode_payld_t;   /**< Parse tree node type-specific payload */
typedef struct ptree_tnode_payld  ptree_tnode_payld_t;   /**< Parse tree terminal node payload      */
typedef struct ptree_ntnode_payld ptree_ntnode_payld_t;  /**< Parse tree non-terminal node payload  */

#endif /* end of #ifndef CTXFryer__ptree_types_h */
