#ifndef CTXFryer__ptree_h
#define CTXFryer__ptree_h

/**
 *  \brief  Parse tree
 *
 *  Source file is parsed accordingly to the language grammar
 *  producing parse tree structure (instantiation of the grammar
 *  tree structure).
 *
 *  Parse tree nodes are only added during successful parsing
 *  (i.e. the parse tree instance only grows).
 *  Therefore, this implementation always allocates a pack
 *  of nodes (if required) to decrease amount of allocations
 *  necessary (the object pack module is used for implementation).
 *
 *  Each of the parse tree nodes has a pointer to its parent,
 *  both its siblings and the 1st child (if any, of course).
 *  1st child's pointer to previous sibling should therefore
 *  be \c NULL.
 *  However, to save space and have access to the sibling list
 *  tail at the same time, the 1st child's pointer to previous
 *  sibling actualy points to the last sibling.
 *  The last sibling's pointer to next sibling is \c NULL, though.
 *  This partially cyclic structure allows for both efficient
 *  sibling access and easy detection of the sibling list end.
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2012/06/15
 */

#include "ptree_types.h"
#include "objpack.h"
#include "lexical_analyser.h"
#include "grammar.h"
#include "attribute.h"

#include <stddef.h>


/** Parse tree terminal node payload */
struct ptree_tnode_payld {
    la_item_t item;  /**< Lexical item */
};  /* end of struct ptree_tnode_payld */


/** Parse tree non-terminal node payload */
struct ptree_ntnode_payld {
    const grammar_rule_t *rule;  /**< Grammar rule */
};  /* end of struct ptree_ntnode_payld */


/** Parse tree node type-specific payload */
union ptree_snode_payld {
    ptree_tnode_payld_t  tnode;   /**< Parse tree terminal node payload     */
    ptree_ntnode_payld_t ntnode;  /**< Parse tree non-terminal node payload */
};  /* end of union ptree_snode_payld */


/** Parse tree node payload */
struct ptree_node_payld {
    attr_handles_t        *attrs;     /**< Grammar symbol attributes          */
    size_t                 attr_cnt;  /**< Attribute count                    */
    size_t                 agra_cnt;  /**< Aggregated attribute count         */
    const attr_name_fsa_t *attr_res;  /**< Attribute resolver (by name)       */
    ptree_snode_payld_t    spec;      /**< P. tree node type-specific payload */
};  /* end of struct ptree_node_payld */


/** Parse tree node */
struct ptree_node {
    ptree_node_type_t   type;       /**< Node type         */
    ptree_node_t       *parent;     /**< Node parent       */
    ptree_node_t       *child;      /**< Node 1st child    */
    size_t              child_cnt;  /**< Children count    */
    ptree_node_t       *next;       /**< Next     sibling  */
    ptree_node_t       *prev;       /**< Previous sibling  */
    objpack_handle_t   *pack;       /**< The node pack     */
    ptree_node_payld_t  payld;      /**< The node payload  */
};  /* end of struct ptree_node */


/**
 *  \brief  Parse tree node type getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Parse tree node type
 */
#define ptree_node_type(node) ((node)->type)


/**
 *  \brief  Check if parse tree node is a terminal node
 *
 *  \param  node  Parse tree node
 *
 *  \retval Non-zero if the node is terminal
 *  \retval 0        otherwise
 */
#define ptree_node_is_terminal(node) (PTREE_TNODE == ptree_node_type(node))


/**
 *  \brief  Check if parse tree node is a non-terminal node
 *
 *  \param  node  Parse tree node
 *
 *  \retval Non-zero if the node is non-terminal
 *  \retval 0        otherwise
 */
#define ptree_node_is_nonterminal(node) (PTREE_NTNODE == ptree_node_type(node))


/**
 *  \brief  Parse tree terminal node lexical item getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Lexical item or \c NULL if the node isn't a terminal node
 */
#define ptree_tnode_item(node) \
    (ptree_node_is_terminal(node) ? &(node)->payld.spec.tnode.item : NULL)


/**
 *  \brief  Parse tree non-terminal node grammar rule getter
 *
 *  The macro expands to grammar rule that was the model
 *  for creation of the parse tree node.
 *  The rule was used for derivation that produced the node.
 *
 *  \param  node  Parse tree node
 *
 *  \return Grammar rule number or \c NULL if the node isn't a non-terminal node
 */
#define ptree_ntnode_rule(node) \
    (ptree_node_is_nonterminal(node) ? NULL : (node)->payld.spec.ntnode.rule)


/**
 *  \brief  Parse tree node parent getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Node parent in the parse tree or \c NULL if it's the root node
 */
#define ptree_node_get_parent(node) ((node)->parent)


/**
 *  \brief  Parse tree node parent setter
 *
 *  \param  node  Parse tree node
 *  \param  prnt  Parse tree node parent
 *
 *  \return Node parent in the parse tree or \c NULL if it's the root node
 */
#define ptree_node_set_parent(node, prnt) ((node)->parent = (prnt))


/**
 *  \brief  Parse tree node children list getter
 *
 *  \param  node  Parse tree node
 *
 *  \return 1st child or \c NULL if \c node is a leaf
 */
#define ptree_node_get_child(node) ((node)->child)


/**
 *  \brief  Parse tree node children count getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Children count
 */
#define ptree_node_get_child_count(node) ((node)->child_cnt)


/**
 *  \brief  Parse tree node children list setter
 *
 *  \param  node  Parse tree node
 *  \param  chld  Parse tree node 1st child
 *
 *  \return 1st child or \c NULL if \c node is a leaf
 */
#define ptree_node_set_child(node, chld, chld_cnt) \
    do { \
        (node)->child     = (chld); \
        (node)->child_cnt = (chld_cnt); \
    } while (0)


/**
 *  \brief  Parse tree node next sibling getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Next sibling or \c NULL if it's the last child
 */
#define ptree_node_get_next(node) ((node)->next)


/**
 *  \brief  Parse tree node next sibling setter
 *
 *  \param  node  Parse tree node
 *  \param  nxt   Parse tree node next sibling
 *
 *  \return Next sibling or \c NULL if it's the last child
 */
#define ptree_node_set_next(node, nxt) ((node)->next = (nxt))


/**
 *  \brief  Parse tree node previous sibling getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Previous sibling or \c NULL if it's the 1st child
 */
#define ptree_node_get_prev(node) ((node)->prev)


/**
 *  \brief  Parse tree node previous sibling setter
 *
 *  \param  node  Parse tree node
 *  \param  prv   Parse tree node previous sibling
 *
 *  \return Previous sibling or \c NULL if it's the 1st child
 */
#define ptree_node_set_prev(node, prv) ((node)->prev = (prv))


/**
 *  \brief  Parse tree node attributes getter
 *
 *  \param  node  Parse tree node
 *
 *  \return Attributes
 */
#define ptree_node_get_attrs(node) ((node)->payld.attrs)


/**
 *  \brief  Parse tree node attributes setter
 *
 *  \param  node  Parse tree node
 *  \param  atrs  Parse tree node attributes
 *
 *  \return Attributes
 */
#define ptree_node_set_attrs(node, atrs) ((node)->payld.attrs = (atrs))


/**
 *  \brief  Parse tree node attribute resolving (by name)
 *
 *  \param  node  Parse tree node
 *  \param  name  Attribute name
 *
 *  \return Attribute or \c NULL unless exists
 */
#define ptree_get_attr(node, name) \
    (assert(NULL != (node)), \
     attr_get((node)->payld.attrs, (node)->payld.attr_res, (name)))


/**
 *  \brief  Parse tree terminal node constructor
 *
 *  \param  item          Lexical item
 *  \param  attr_cnt      Count of attributes
 *  \param  attr_classes  Attribute classes (aggreg. attrs first)
 *  \param  g_attr_evals  Generic attribute evaluators
 *  \param  s_attr_evals  Specific attribute evaluators
 *  \param  attr_res      Attribute resolver (by name), opitional
 *
 *  \return Parse tree node or \c NULL in case of memory error
 */
ptree_node_t *ptree_tnode_create(
    const la_item_t              *item,
    size_t                        attr_cnt,
    const attr_class_descrs_t    *attr_classes,
    const attr_eval_descr_refs_t *g_attr_evals,
    const attr_eval_descr_refs_t *s_attr_evals,
    const attr_name_fsa_t        *attr_res);


/**
 *  \brief  Parse tree non-terminal node constructor
 *
 *  \param  rule          Grammar rule
 *  \param  attr_cnt      Count of attributes
 *  \param  agra_cnt      Count of aggregated attributes
 *  \param  attr_classes  Attribute classes (aggreg. attrs first)
 *  \param  g_attr_evals  Generic attribute evaluators
 *  \param  s_attr_evals  Specific attribute evaluators
 *  \param  attr_res      Attribute resolver (by name), opitional
 *
 *  \return Parse tree node or \c NULL in case of memory error
 */
ptree_node_t *ptree_ntnode_create(
    const grammar_rule_t         *rule,
    size_t                        attr_cnt,
    const attr_class_descrs_t    *attr_classes,
    const attr_eval_descr_refs_t *g_attr_evals,
    const attr_eval_descr_refs_t *s_attr_evals,
    const attr_name_fsa_t        *attr_res);


/**
 *  \brief  Set attribute evaluators of symbol assocated with parse tree node
 *
 *  \param  node        Parse tree node
 *  \param  attr_evals  Attribute evaluators
 *
 *  \retval non-zero in case of success
 *  \retval 0        otherwise (memory error)
 */
int ptree_node_attr_evaluators(
    ptree_node_t                 *node,
    const attr_eval_descr_refs_t *attr_evals);


/**
 *  \brief  Resolve (link) symbol attribute dependencies
 *
 *  The function sets attribute dependencies based on
 *  attribute evaluators set.
 *
 *  \param  node  Parse tree node (with children set if any)
 *  \param  type  Attributes type (aggregated/inherited)
 *
 *  \retval non-zero in case of success
 *  \retval 0        otherwise (memory error)
 */
int ptree_resolve_attr_dependencies(ptree_node_t *node, attr_type_t type);


/**
 *  \brief  Evaluate parse tree node attributes
 *
 *  The function resolves attributes by their names and evaluates them
 *  (unless already evaluated).
 *  The attribute names are provided via in/out arguments of variable count,
 *  formally of type \c void \c **.
 *  As input, they are treated as pointers to \c const \c char \c *
 *  (C-string containing attribute name).
 *  The last one MUST be set to \c NULL.
 *  As output, their target is set to respective attribute value
 *  if the attribute was evaluated, otherwise set to \c NULL.
 *  The function returns the cumulative evaluation status (the worst status
 *  spotted during the evaluation).
 *
 *  \param[in]      node  Parse tree node
 *  \param[in,out]  ...   Attribute names (C-strings) / value refs.
 *
 *  \retval ATTR_EVAL_OK     in case all evaluations were sucessfull
 *  \retval ATTR_EVAL_UNDEF  if at least one attribute value could not be defined
 *  \retval ATTR_EVAL_ERROR  in case of any evaluation failure
 *                           (or if any of the attributes doesn't exist)
 */
attr_eval_status_t ptree_node_attrs_eval(ptree_node_t *node, ...);


/**
 *  \brief  Evaluate parse tree node attribute
 *
 *  The function restricts the \ref ptree_node_attrs_eval to just
 *  one attribute (so that the parameters don't need to be pointers
 *  to pointers).
 *
 *  \param[in]   node       Parse tree node
 *  \param[in]   attr_name  Attribute name (C-string)
 *  \param[out]  value      Attribute value (optional, \c NULL is accepted)
 *
 *  \retval ATTR_EVAL_OK     in case the evaluation was sucessfull
 *  \retval ATTR_EVAL_UNDEF  if the attribute value could not be defined
 *  \retval ATTR_EVAL_ERROR  in case of evaluation failure
 *                           (or if the attribute doesn't exist)
 */
attr_eval_status_t ptree_node_attr_eval(
    ptree_node_t  *node,
    const char    *attr_name,
    void         **value);


/**
 *  \brief  Parse tree node destructor
 *
 *  \param  node  Parse tree node
 */
void ptree_node_destroy(ptree_node_t *node);


/**
 *  \brief  Parse tree destructor
 *
 *  The function destroys the whole parse tree handed-in
 *  by its root node.
 *
 *  Note that the function may actually be used on any
 *  parse tree node (in which case the sub-tree would be
 *  destroyed).
 *  In such case, however, it's the caller's responsibility
 *  to fix the parse tree so that the destroued sub-tree
 *  root (i.e. the \c root parameter) is no longer used.
 *
 *  \param  root  Parse tree root
 */
void ptree_destroy(ptree_node_t *root);

#endif /* end of #ifndef CTXFryer__ptree_h */
