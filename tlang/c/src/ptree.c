/**
 *  \brief  Parse tree
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2012/09/19
 */

#include "ptree.h"
#include "attribute.h"
#include "objpack.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#define PTREE_NODE_PACK_CAPACITY       256  /**< Tree node pack capacity       */
#define PTREE_NODE_PACK_CACHE_MAX        1  /**< Tree node pack cache max      */
#define PTREE_NODE_REF_PACK_CAPACITY   256  /**< Tree node ref. pack capacity  */
#define PTREE_NODE_REF_PACK_CACHE_MAX    1  /**< Tree node ref. pack cache max */


/** Parse tree node pack pool */
static objpack_pool_t ptree_node_pack_pool = OBJPACK_POOL_INIT(
    sizeof(ptree_node_t), PTREE_NODE_PACK_CAPACITY, PTREE_NODE_PACK_CACHE_MAX);


/** Parse tree node reference pack pool */
static objpack_pool_t ptree_node_ref_pack_pool = OBJPACK_POOL_INIT(
    sizeof(ptree_node_t *), PTREE_NODE_REF_PACK_CAPACITY, PTREE_NODE_REF_PACK_CACHE_MAX);


/*
 * Static functions declarations
 */

static ptree_node_t *ptree_node_create(
    ptree_node_type_t             type,
    size_t                        attr_cnt,
    const attr_class_descrs_t    *attr_classes,
    const attr_eval_descr_refs_t *g_attr_evals,
    const attr_eval_descr_refs_t *s_attr_evals,
    const attr_name_fsa_t        *attr_res);

inline static void ptree_resolve_attr_deps_aggreg(
    ptree_node_t  *node,
    ptree_node_t **node_refs,
    size_t         node_cnt);

inline static void ptree_resolve_attr_deps_inher(
    ptree_node_t  *node,
    ptree_node_t **node_refs,
    size_t         node_cnt);

static void ptree_resolve_attr_deps(
    attr_handle_t  *attr,
    ptree_node_t  **node_refs,
    size_t          node_cnt);

inline static void ptree_tnode_finalise(ptree_node_t *node);

inline static void ptree_ntnode_finalise(ptree_node_t *node);

static void ptree_finalise(void) __attribute__((destructor));


/*
 * Interface implementation
 */

ptree_node_t *ptree_tnode_create(
    const la_item_t              *item,
    size_t                        attr_cnt,
    const attr_class_descrs_t    *attr_classes,
    const attr_eval_descr_refs_t *g_attr_evals,
    const attr_eval_descr_refs_t *s_attr_evals,
    const attr_name_fsa_t        *attr_res)
{
    ptree_node_t *node = ptree_node_create(
        PTREE_TNODE,
        attr_cnt, attr_classes, g_attr_evals, s_attr_evals, attr_res);

    if (NULL == node) return NULL;

    la_item_copy(&node->payld.spec.tnode.item, item);

    return node;
}


ptree_node_t *ptree_ntnode_create(
    const grammar_rule_t         *rule,
    size_t                        attr_cnt,
    const attr_class_descrs_t    *attr_classes,
    const attr_eval_descr_refs_t *g_attr_evals,
    const attr_eval_descr_refs_t *s_attr_evals,
    const attr_name_fsa_t        *attr_res)
{
    ptree_node_t *node = ptree_node_create(
        PTREE_NTNODE,
        attr_cnt, attr_classes, g_attr_evals, s_attr_evals, attr_res);

    if (NULL == node) return NULL;

    node->payld.spec.ntnode.rule = rule;

    return node;
}


int ptree_node_attr_evaluators(
    ptree_node_t                 *node,
    const attr_eval_descr_refs_t *attr_evals)
{
    assert(NULL != node);

    return ATTR_EVAL_OK == attr_evaluators(
        node->payld.attrs, node->payld.attr_cnt, attr_evals);
}


int ptree_resolve_attr_dependencies(ptree_node_t *node, attr_type_t type) {
    assert(NULL != node);

    /* Allocate temporary node reference pack */
    objpack_handle_t *node_refs_pack;
    size_t            child_cnt = node->child_cnt;

    ptree_node_t **node_refs = (ptree_node_t **)objpack_alloc(
        &ptree_node_ref_pack_pool, child_cnt, &node_refs_pack);

    if (NULL == node_refs) return 0;

    /* Set node as symbol with index 0 */
    node_refs[0] = node;

    /* Set node children at the next positions */
    ptree_node_t *child    = node->child;
    size_t        node_idx = 1;

    for (; node_idx <= child_cnt; ++node_idx) {
        assert(NULL != child);

        node_refs[node_idx] = child;

        child = child->next;
    }

    /* All children should've been set */
    assert(NULL == child);

    /* Perform dependencies resolution */
    size_t node_cnt = child_cnt + 1;

    switch (type) {
        case ATTR_TYPE_AGGREGATED:
            ptree_resolve_attr_deps_aggreg(
                node_refs[0], node_refs, node_cnt);

            break;

        case ATTR_TYPE_INHERITED:
            for (node_idx = 1; node_idx < node_cnt; ++node_idx)
                ptree_resolve_attr_deps_inher(
                    node_refs[node_idx], node_refs, node_cnt);

            break;
    }

    /* Free node reference pack */
    objpack_unref(node_refs_pack);

    /* Done */
    return 1;
}


attr_eval_status_t ptree_node_attrs_eval(ptree_node_t *node, ...) {
    assert(NULL != node);

    attr_eval_status_t status = ATTR_EVAL_OK;

    va_list args;

    va_start(args, node);

    for (;;) {
        void **arg = va_arg(args, void **);

        if (NULL == arg) break;

        /* Resolve the attribute */
        attr_handle_t *attr = ptree_get_attr(node, *((const char **)arg));

        if (NULL == attr)
            status = ATTR_EVAL_ERROR;  /* Worst that can happen */

        /* Evaluate the attribute */
        /* TODO: The dependency depth could be limited; but what with? */
        attr_eval_status_t eval_status = attr_eval(attr, 0);

        if (ATTR_EVAL_OK == eval_status)
            *arg = attr_get_value(attr);

        /* Evaluation wasn't successful */
        else {
            *arg = NULL;

            if (status < eval_status)
                status = eval_status;
        }
    }

    va_end(args);

    return status;
}


attr_eval_status_t ptree_node_attr_eval(
    ptree_node_t  *node,
    const char    *attr_name,
    void         **value)
{
    void *name_val = (void *)attr_name;

    attr_eval_status_t status = ptree_node_attrs_eval(node, &attr_name, NULL);

    if (NULL != value)
        *value = name_val;

    return status;
}


void ptree_node_destroy(ptree_node_t *node) {
    assert(NULL != node);

    /* Destroy attributes */
    attr_destroy(node->payld.attrs, node->payld.attr_cnt);

    /* Finalise the node */
    switch (ptree_node_type(node)) {
        case PTREE_TNODE:
            ptree_tnode_finalise(node);

            break;

        case PTREE_NTNODE:
            ptree_ntnode_finalise(node);

            break;
    }

    objpack_unref(node->pack);
}


void ptree_destroy(ptree_node_t *root) {
    assert(NULL != root);

    /* Tear the root node off any possible sibling list */
    root->prev = root->next = NULL;

    /* Sentence the gloomy verdict */
    ptree_node_t *death_wrnt = root;

    /* Execute the condamned */
    while (NULL != death_wrnt) {
        ptree_node_t *dead = death_wrnt;

        /* The ancestors shall be slaughtered, too, in a moment... */
        if (NULL != dead->child) {
            assert(NULL != dead->child->prev);

            dead->child->prev->next = dead->next;
            death_wrnt = dead->child;
        }
        else
            death_wrnt = dead->next;

        /* ... but they have to watch as we swing the axe ;-) */
        ptree_node_destroy(dead);
    }
}


/*
 * Static functions definitions
 */

/**
 *  \brief  Parse tree node constructor
 *
 *  The function creates parse tree node associated with a grammar symbol.
 *
 *  \param  type          Parse tree node type
 *  \param  attr_cnt      Number of the node symbol attributes
 *  \param  attr_classes  Node symbol attributes classes
 *  \param  g_attr_evals  Node symbol attributes generic evaluators
 *  \param  s_attr_evals  Node symbol attributes specific evaluators
 *  \param  attr_res      Attribute resolver (by name), opitional
 *
 *  \return Parse tree node (partially initialised, only)
 *          or \c NULL in case of memory error
 */
static ptree_node_t *ptree_node_create(
    ptree_node_type_t             type,
    size_t                        attr_cnt,
    const attr_class_descrs_t    *attr_classes,
    const attr_eval_descr_refs_t *g_attr_evals,
    const attr_eval_descr_refs_t *s_attr_evals,
    const attr_name_fsa_t        *attr_res)
{
    /* Parse tree node allocation */
    objpack_handle_t *node_pack;

    ptree_node_t *node = (ptree_node_t *)objpack_alloc(
        &ptree_node_pack_pool, 1, &node_pack);

    if (NULL == node) return NULL;

    /* Parse tree structure initialisation */
    node->type      = type;
    node->parent    = NULL;
    node->child     = NULL;
    node->child_cnt = 0;
    node->next      = NULL;
    node->prev      = NULL;
    node->pack      = node_pack;  /* the node keeps my pack reference */

    memset(&node->payld, 0, sizeof(ptree_node_payld_t));

    /* Create associated symbol attributes */
    if (attr_cnt) {
        node->payld.attr_cnt = attr_cnt;

        node->payld.attrs = attr_create(&node->payld.agra_cnt,
            node, attr_cnt, attr_classes, g_attr_evals, s_attr_evals);

        if (NULL == node->payld.attrs) {
            objpack_unref(node->pack);

            return NULL;
        }
    }

    /* Set attribute resolver (by name) */
    node->payld.attr_res = attr_res;

    return node;
}


/**
 *  \brief  Perform aggregated attributes dependencies resolution
 *
 *  The function resolves (links) a symbol aggregated attributes dependencies
 *  based on evaluation descriptors.
 *
 *  The function accepts an array of parse tree nodes.
 *  They represent grammar symbols in a rule.
 *  The rule LHS is at index 0, the rest are RHS symbols
 *  in the order they appear in the rule.
 *  This is consistent with dependency node index of the evaluation
 *  descriptor.
 *
 *  \param  node       Parse tree node
 *  \param  node_refs  Parse tree node references
 *  \param  node_cnt   Count of parse tree node references
 */
inline static void ptree_resolve_attr_deps_aggreg(
    ptree_node_t  *node,
    ptree_node_t **node_refs,
    size_t         node_cnt)
{
    assert(NULL != node);

    size_t attr_idx = 0;
    size_t attr_cnt = node->payld.agra_cnt;

    for (; attr_idx < attr_cnt; ++attr_idx)
        ptree_resolve_attr_deps((*node->payld.attrs) + attr_idx,
                                node_refs, node_cnt);
}


/**
 *  \brief  Perform inherited attributes dependencies resolution
 *
 *  The function resolves (links) a symbol inherited attributes dependencies
 *  based on evaluation descriptors.
 *
 *  The function accepts an array of parse tree nodes.
 *  They represent grammar symbols in a rule.
 *  The rule LHS is at index 0, the rest are RHS symbols
 *  in the order they appear in the rule.
 *  This is consistent with dependency node index of the evaluation
 *  descriptor.
 *
 *  \param  node       Parse tree node
 *  \param  node_refs  Parse tree node references
 *  \param  node_cnt   Count of parse tree node references
 */
inline static void ptree_resolve_attr_deps_inher(
    ptree_node_t  *node,
    ptree_node_t **node_refs,
    size_t         node_cnt)
{
    assert(NULL != node);

    size_t attr_idx = node->payld.agra_cnt;
    size_t attr_cnt = node->payld.attr_cnt;

    for (; attr_idx < attr_cnt; ++attr_idx)
        ptree_resolve_attr_deps((*node->payld.attrs) + attr_idx,
                                node_refs, node_cnt);
}


/**
 *  \brief  Perform attribute dependencies resolution
 *
 *  The function resolves (links) attribute dependencies based on
 *  its evaluation descriptor.
 *
 *  \param  attr       Attribute
 *  \param  node_refs  Parse tree node references
 *  \param  node_cnt   Count of parse tree node references
 */
static void ptree_resolve_attr_deps(
    attr_handle_t  *attr,
    ptree_node_t  **node_refs,
    size_t          node_cnt)
{
    assert(NULL != attr);

    /* Dependencies must be allocated */
    assert(attr_is_depend_init(attr));

    /* Skip already resolved dependencies */
    if (attr_is_depend_resolved(attr)) return;

    /* Set dependencies by evaluation descriptor */
    size_t dep_idx = 0;

    for (; dep_idx < attr_get_depend_count(attr); ++dep_idx) {
        size_t dep_node_idx = attr_get_depend_symbol_index(attr, dep_idx);

        assert(dep_node_idx < node_cnt);

        ptree_node_t *dep_node = node_refs[dep_node_idx];

        size_t dep_attr_idx = attr_get_depend_attr_index(attr, dep_idx);

        assert(dep_attr_idx < dep_node->payld.attr_cnt);

        attr_handle_t *dep_attr = (*dep_node->payld.attrs) + dep_attr_idx;

        attr_set_depend(attr, dep_idx, dep_attr);
    }

    /* Set dependencies status */
    attr_set_depend_resolved(attr);
}


/**
 *  \brief  Finalise parse tree terminal node
 *
 *  \param  node  Parse tree node
 */
inline static void ptree_tnode_finalise(ptree_node_t *node) {
    assert(NULL != node);

    la_item_destroy(&node->payld.spec.tnode.item);
}


/**
 *  \brief  Finalise parse tree non-terminal node
 *
 *  \param  node  Parse tree node
 */
inline static void ptree_ntnode_finalise(ptree_node_t *node) {
    assert(NULL != node);

    /* Nothing to do */
}


/**
 *  \brief  Module destructor (GCC only!)
 *
 *  If compiled with gcc this shall be executed after \c main
 *  routine scope is left.
 *
 *  Finalises node pack pool.
 */
static void ptree_finalise(void) {
    objpack_finalise(&ptree_node_pack_pool);
    objpack_finalise(&ptree_node_ref_pack_pool);
}
