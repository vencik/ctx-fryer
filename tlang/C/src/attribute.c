/**
 *  \brief  Attribute
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/06/29
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

#include "attribute.h"
#include "objpack.h"
#include "stack.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define ATTR_HANDLE_PACK_CAPACITY      256  /**< Handle pack capacity     */
#define ATTR_HANDLE_PACK_CACHE_MAX       1  /**< Handle pack cache max    */

#define ATTR_REF_PACK_CAPACITY         512  /**< Reference pack capacity  */
#define ATTR_REF_PACK_CACHE_MAX         16  /**< Reference pack cache max */


/** Attribute pack pool */
static objpack_pool_t attr_handle_pack_pool = OBJPACK_POOL_INIT(
    sizeof(attr_handle_t),
    ATTR_HANDLE_PACK_CAPACITY,
    ATTR_HANDLE_PACK_CACHE_MAX);


/** Attribute reference pack pool */
static objpack_pool_t attr_ref_pack_pool = OBJPACK_POOL_INIT(
    sizeof(attr_handle_t *),
    ATTR_REF_PACK_CAPACITY,
    ATTR_REF_PACK_CACHE_MAX);


static attr_eval_status_t attr_eval_stack(stack_t *stack);

static ssize_t attr_resolve_index(const attr_name_fsa_t *name_fsa, const char *name);

static void attr_finalise(void) __attribute__((destructor));


attr_handles_t *attr_create(
    size_t                       *aggreg_attr_cnt,
    ptree_node_t                 *ptnode,
    size_t                        cnt,
    const attr_class_descrs_t    *classes,
    const attr_eval_descr_refs_t *g_evals,
    const attr_eval_descr_refs_t *s_evals)
{
    assert(NULL != aggreg_attr_cnt);
    assert(NULL != ptnode);
    assert(NULL != classes);

    *aggreg_attr_cnt = 0;

    if (0 == cnt) return NULL;

    /* Allocate attributes pack */
    objpack_handle_t *attr_pack;

    attr_handles_t *attrs = (attr_handles_t *)objpack_alloc(
        &attr_handle_pack_pool, cnt, &attr_pack);

    if (NULL == attrs) return NULL;

    /* Initialise attributes */
    int inherited_spotted = 0;

    size_t i = 0;

    for (; i < cnt; ++i) {
        attr_handle_t            *attr    = *attrs + i;
        const attr_class_descr_t *cla2s   = *classes + i;
        const attr_eval_descr_t  *eval    = NULL;
        size_t                    dep_cnt = 0;

        /* Check attributes (meta) type order & count aggregated */
        switch (cla2s->type) {
            case ATTR_TYPE_AGGREGATED:
                assert(!inherited_spotted);

                ++*aggreg_attr_cnt;

                break;

            case ATTR_TYPE_INHERITED:
                inherited_spotted = 1;

                break;
        }

        /* Specific evaluator is preferred */
        if (NULL != s_evals)
            eval = (*s_evals)[i];

        /* Generic is the fallback */
        if (NULL == eval) {
            if (NULL != g_evals)
                eval = (*g_evals)[i];
        }

        /* Allocate dependencies pack */
        objpack_handle_t   *dep_pack = NULL;
        attr_handle_refs_t *deps     = NULL;

        if (NULL != eval) {
            dep_cnt = eval->dep_cnt;

            deps = (attr_handle_refs_t *)objpack_calloc(
                       &attr_ref_pack_pool, dep_cnt, &dep_pack);

            if (NULL == deps) {
                /* Release dependencies of attributes initialised so far */
                while (i--) {
                    dep_pack = (*attrs)[i].dep_pack;

                    if (NULL != dep_pack)
                        objpack_unref(dep_pack);
                }

                objpack_unref(attr_pack);  /* Release attributes */

                return NULL;
            }

            assert(NULL != dep_pack);
        }

        /* Initialise attribute */
        attr->st_flags    = 0x00000000;
        attr->ptnode      = ptnode;
        attr->pack        = attr_pack;
        attr->class_descr = cla2s;
        attr->eval_descr  = eval;
        attr->val         = NULL;
        attr->dep_pack    = dep_pack;
        attr->deps        = deps;  /* owned reference passed to attr. */

        attr_set_eval_status(attr, ATTR_EVAL_UNDEF);

        /* Dependencies were not allocated iff evaluator is unknown */
        assert((NULL != eval) ^ (NULL == deps));

        if (NULL != deps) {
            attr_set_depend_init(attr);

            /* Zero dependencies are trivially resolved & evaluated */
            if (0 == dep_cnt)
                _attr_set_depend_flags(attr, ATTR_DEPS_RESOLVED | ATTR_DEPS_EVALUATED);
        }
    }

    /* Mark attributes references to the pack and drop own one */
    objpack_ref_by(attr_pack, cnt - 1);

    return attrs;
}


void attr_destroy(attr_handles_t *attrs, size_t cnt) {
    assert(NULL != attrs || cnt == 0);

    size_t i = 0;

    for (; i < cnt; ++i) {
        attr_handle_t *attr = (*attrs) + i;

        /* Destroy value... */
        if (ATTR_EVAL_OK == attr_get_eval_status(attr))
            /* ... but not if the attribute is a reference */
            if (!attr_is_reference(attr)) {
                attr_destructor_t *destroy = attr->class_descr->destroy;

                if (NULL != destroy)
                    destroy(attr);
            }

        /* Drop reference to the dependencies pack */
        if (NULL != attr->dep_pack)
            objpack_unref(attr->dep_pack);

        /* Drop reference to the attribute pack */
        objpack_unref(attr->pack);
    }
}


attr_eval_status_t attr_evaluators(
    attr_handles_t               *attrs,
    size_t                        cnt,
    const attr_eval_descr_refs_t *evals)
{
    assert(NULL != attrs || cnt == 0);

    /* Nothing to do */
    if (NULL == evals) return ATTR_EVAL_OK;

    /* Set evaluators */
    size_t i = 0;

    for (; i < cnt; ++i) {
        attr_handle_t           *attr    = (*attrs) + i;
        const attr_eval_descr_t *eval    = (*evals)[i];
        size_t                   dep_cnt = 0;

        /* No setting */
        if (NULL == eval) continue;

        /* Allocate dependencies pack */
        dep_cnt = eval->dep_cnt;

        objpack_handle_t *dep_pack;

        attr_handle_refs_t *deps = (attr_handle_refs_t *)objpack_calloc(
            &attr_ref_pack_pool, dep_cnt, &dep_pack);

        if (NULL == deps) return ATTR_EVAL_ERROR;

        /* Get rid of previous dependencies pack */
        if (NULL != attr->dep_pack)
            objpack_unref(attr->dep_pack);

        attr_clear_depend_flags(attr);

        /* Set attribute evaluator & dependencies pack */
        attr->eval_descr = eval;
        attr->dep_pack   = dep_pack;
        attr->deps       = deps;  /* owned reference passed to attr. */

        attr_set_depend_init(attr);

        /* Zero dependencies are trivially resolved & evaluated */
        if (0 == dep_cnt)
            _attr_set_depend_flags(attr, ATTR_DEPS_RESOLVED | ATTR_DEPS_EVALUATED);

        assert(ATTR_EVAL_UNDEF == attr_get_eval_status(attr));
    }

    return ATTR_EVAL_OK;
}


attr_eval_status_t attr_eval(attr_handle_t *attr, size_t depth_max) {
    assert(NULL != attr);

    /* Check current attribute evaluation status */
    attr_eval_status_t status = attr_get_eval_status(attr);

    if (ATTR_EVAL_UNDEF != status) return status;

    /* Create attribute ref. stack */
    stack_t stack;

    stack_init(&stack, &attr_ref_pack_pool,
        depth_max ? depth_max : STACK_SIZE_MAX);

    /* Initialise stack top with the evaluated attribute */
    attr_handle_t **attr_item = (attr_handle_t **)stack_push(&stack);

    if (NULL == attr_item) return ATTR_EVAL_ERROR;

    *attr_item = attr;

    /* Evaluate all attributes on stack */
    status = attr_eval_stack(&stack);

    /* Cleanup attribute ref. stack */
    while (!stack_empty(&stack)) {
        attr_item = (attr_handle_t **)stack_top(&stack);

        assert(NULL != attr_item);

        attr_clear_depend_eval_scheduled(*attr_item);

        stack_pop(&stack);
    }

    return status;
}


attr_handle_t *attr_get(
    attr_handles_t        *attrs,
    const attr_name_fsa_t *name_fsa,
    const char            *name)
{
    if (NULL == name_fsa) return NULL;

    ssize_t idx = attr_resolve_index(name_fsa, name);

    if (-1 == idx) return NULL;

    assert(NULL != attrs);

    return *attrs + idx;
}


/**
 *  \brief  Evaluate attributes on stack
 *
 *  The function implements \ref attr_eval deep on-demand evaluation loop.
 *
 *  It's made a separate function mainly from pragmatic reason of
 *  allowing early break of the loop using mere \c return (while
 *  the stack shall be cleaned up by the caller).
 *
 *  \param  stack  Attribute stack
 *
 *  \return See \ref attr_eval
 */
static attr_eval_status_t attr_eval_stack(stack_t *stack) {
    assert(NULL != stack);

    attr_eval_status_t status = ATTR_EVAL_UNDEF;

    do {
        /* Get stack top attribute */
        attr_handle_t **attr_item = (attr_handle_t **)stack_top(stack);

        assert(NULL != attr_item);

        attr_handle_t *attr = *attr_item;

        assert(NULL != attr);

        assert(NULL != attr->eval_descr);
        assert(NULL != attr->deps);  /* non-NULL even for nullary attrs */

        /* Dependencies must be resolved at this point in order to continue */
        if (!attr_is_depend_resolved(attr)) return ATTR_EVAL_UNDEF;

        /* Check dependencies evaluation status */
        int deps_ready = 1;

        if (!attr_is_depend_evaluated(attr)) {
            /*
             * Dependencies are checked and pushed to stack in reversed order
             * (so that their evaluation is done in the left-to-right order).
             */
            size_t i = attr->eval_descr->dep_cnt;

            while (i) {
                --i;

                attr_handle_t *dep = (*attr->deps)[i];

                assert(NULL != dep);  /* dependency is resolved */

                /*
                 * Circular dependency detection
                 *
                 * The evaluation is implemented using bredth-first descend
                 * to the dependencies.
                 * That means that at any time, any attributes on the stack
                 * are (transitive) dependencies of all attributes below them
                 * (proof invariant).
                 * As soon as an attribute appears on the stack twice a dependency
                 * loop is detected.
                 */
                if (attr_is_depend_eval_scheduled(dep)) return ATTR_EVAL_ERROR;

                attr_eval_status_t dep_eval_status = attr_get_eval_status(dep);

                /* Dependency needs evaluation */
                if (ATTR_EVAL_UNDEF == dep_eval_status) {
                    attr_item = (attr_handle_t **)stack_push(stack);

                    if (NULL == attr_item) return ATTR_EVAL_ERROR;

                    *attr_item = dep;

                    deps_ready = 0;
                }

                /* Dependency evaluation error */
                else if (ATTR_EVAL_ERROR == dep_eval_status)
                    return ATTR_EVAL_ERROR;

                /* Dependency is already evaluated (sanity check) */
                else
                    assert(ATTR_EVAL_OK == dep_eval_status);
            }

            /*
             * Attr. dependencies are now either evaluated
             * or their evaluation is scheduled (they are
             * on the stack above the attribute)
             */
            if (deps_ready) {
                attr_clear_depend_eval_scheduled(attr);
                attr_set_depend_evaluated(attr);
            }
            else
                attr_set_depend_eval_scheduled(attr);
        }

        /* Evaluate attribute if dependencies are ready */
        if (deps_ready) {
            assert(attr_is_depend_evaluated(attr));

            stack_pop(stack);  /* pop the attribute anyway */

            assert(NULL != attr->eval_descr->eval);

            status = attr->eval_descr->eval(attr, attr->deps);

            /* Unsuccessful evaluation */
            if (ATTR_EVAL_OK != status) break;
        }

    } while (!stack_empty(stack));

    return status;
}


/**
 *  \brief  Attribute index resolver
 *
 *  The function resolves attribute index based on attribute name.
 *
 *  \param  name_fsa  Attribute names FSA
 *  \param  name      Attribute name (C-string)
 *
 *  \return Attribute index or \c -1 if no attribute of such name exists
 */
static ssize_t attr_resolve_index(const attr_name_fsa_t *name_fsa, const char *name) {
    assert(NULL != name);

    for (;;) {
        assert(NULL != name_fsa);

        const attr_name_fsa_node_t *node = &name_fsa->node;

        assert(0 < node->branch_cnt);

        char name_ch = *name;

        /* Terminator */
        if ('\0' == name_ch) {
            const attr_name_fsa_branch_t *branch = *node->branches;

            /* Valid name */
            if ('\0' == branch->character) {
                assert(NULL != branch->sub_fsa);

                return branch->sub_fsa->locator.index;
            }

            /* Invalid name */
            return -1;
        }

        /* Interval bisection search */
        size_t low = 0;
        size_t cnt = node->branch_cnt;

        for (;;) {
            size_t rel_idx = cnt / 2;
            size_t abs_idx = low + rel_idx;

            assert(abs_idx < node->branch_cnt);

            const attr_name_fsa_branch_t *branch = *node->branches + abs_idx;

            char ch = branch->character;

            /* Branch found */
            if (name_ch == ch) {
                name_fsa = branch->sub_fsa;

                break;
            }

            /* Bisection */
            assert(rel_idx < cnt);

            if (name_ch < ch)
                cnt  = rel_idx;
            else {
                low  = abs_idx + 1;
                cnt -= rel_idx + 1;
            }

            /* No way further */
            if (0 == cnt) return -1;
        }

        ++name;
    }

    /* Unreachable code */
    assert(NULL == "INTERNAL ERROR: Unreachable code reached");
    abort();
}


/**
 *  \brief  Module destructor (GCC only!)
 *
 *  If compiled with gcc this shall be executed after \c main
 *  routine scope is left.
 *  Cleans attribute handle and reference pack pools up.
 */
static void attr_finalise(void) {
    objpack_finalise(&attr_handle_pack_pool);
    objpack_finalise(&attr_ref_pack_pool);
}
