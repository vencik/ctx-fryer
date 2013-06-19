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
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2013/03/29
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
