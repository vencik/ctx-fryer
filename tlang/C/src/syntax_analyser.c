/**
 *  \brief  Syntax analyser implementation
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/07/17
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

#include "syntax_analyser.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>


#define SA_STACK_CAP_UNIT 1024  /**< PDA stack capacity unit */


/*
 * Integer stack interface
 */

/**
 *  \brief  Stack depth getter
 *
 *  \param  stack  Stack
 *
 *  \return Actual stack depth
 */
#define sa_stack_depth(stack) ((stack)->depth)


/**
 *  \brief  Stack empty check
 *
 *  \param  stack  Stack
 *
 *  \retval non-zero if the stack isn't empty
 *  \retval 0        otherwise
 */
#define sa_stack_empty(stack) (0 == sa_stack_depth(stack))


/**
 *  \brief  Stack capacity getter
 *
 *  \param  stack  Stack
 *
 *  \return Stack capacity
 */
#define sa_stack_capacity(stack) ((stack)->cap)


/**
 *  \brief  Stack depth limit getter
 *
 *  \param  stack  Stack
 *
 *  \return Stack depth limit
 */
#define sa_stack_limit(stack) ((stack)->limit)


/**
 *  \brief  Stack top getter
 *
 *  \param  stack  Stack
 *
 *  \return Stack top
 */
#define sa_stack_get_top(stack) \
    (assert(0 < (stack)->depth), (stack)->impl[(stack)->depth - 1])


/**
 *  \brief Stack top setter
 *
 *  The macro is equivalent to calling
 *  \c sa_stack_pop(s,1) and then \c sa_stack_push(s,i)
 *  but more efficient...
 *
 *  \param  stack  Stack
 *  \param  item   Stacked item
 */
#define sa_stack_set_top(stack, item) \
    do { \
        assert(0 < (stack)->depth); \
        (stack)->impl[(stack)->depth - 1] = (item); \
    } while (0)


/**
 *  \brief  Pop \c n items from stack top
 *
 *  \param  stack  Stack
 *  \param  n      Pop count
 */
#define sa_stack_pop(stack, n) \
    do { \
        assert((n) <= (stack)->depth); \
        (stack)->depth -= (n); \
    } while (0)


/**
 *  \brief  Push item to stack
 *
 *  \param  stack  Stack
 *  \param  item   Stacked item
 *
 *  \retval 0         on success
 *  \retval EOVERFLOW if stack depth limit would be breached
 *  \retval ENOMEM    on memory error
 */
inline static int sa_stack_push(sa_stack_t *stack, size_t item) {
    assert(NULL != stack);

    /* Stack capacity reached */
    if (!(stack->depth < stack->cap)) {
        /* Stack depth is limited */
        if (stack->limit)
            /* Stack limit was reached */
            if (stack->depth == stack->limit)
                return EOVERFLOW;

        /* Increase capacity (but honour the limit, if set) */
        size_t cap = stack->cap + SA_STACK_CAP_UNIT;

        if (stack->limit && cap > stack->limit) cap = stack->limit;

        /* Reallocate the stack implementation */
        size_t *impl = (size_t *)realloc(stack->impl, cap * sizeof(size_t));

        if (NULL == impl)
            return ENOMEM;

        stack->impl = impl;
        stack->cap  = cap;
    }

    assert(stack->depth < stack->cap);

    /* Push item */
    stack->impl[stack->depth++] = item;

    return 0;
}


/**
 *  \brief  Stack constructor
 *
 *  The function creates new stack with initial capacity
 *  of \c cap_mul times \c SA_STACK_CAP_UNIT, that is
 *  if the capacity doesn't breach the stack depth limit
 *  requested.
 *
 *  \param  stack    Stack object
 *  \param  cap_mul  Stack initial capacity multiplier
 *  \param  limit    Stack depth limit (0 means unlimited)
 *
 *  \retval 0      on success
 *  \retval EINVAL if arguments are invalid
 *  \retval ENOMEM on memory error
 */
static int sa_stack_create(sa_stack_t *stack, size_t cap_mul, size_t limit) {
    assert(NULL != stack);

    /* Initial capacity */
    size_t cap = SA_STACK_CAP_UNIT * cap_mul;

    /* Check limit */
    if (limit && cap > limit) return EINVAL;

    /* Allocate stack implementation */
    size_t *impl = (size_t *)malloc(cap * sizeof(size_t));

    if (NULL == impl) return ENOMEM;

    /* Initialise stack */
    stack->depth = 0;
    stack->impl  = impl;
    stack->cap   = cap;
    stack->limit = limit;

    return 0;
}


/**
 *  \brief  Syntax analyser PDA stack destructor
 *
 *  \param  stack  Stack
 */
static void sa_stack_destroy(sa_stack_t *stack) {
    assert(NULL != stack);

    free(stack->impl);
}


/*
 * Parse tree constructors wrappers
 * (checking if parse tree construction is enabled)
 */

/**
 *  \brief  Add parse tree leaf terminal node
 *
 *  \ref sa_ptree_add_tnode_impl wrapper.
 *
 *  \param  sa    Syntax analyser
 *  \param  item  Lexical item
 */
#define sa_ptree_add_tnode(sa, item) \
    do { \
        assert(NULL != (sa)); \
        if ((sa)->ptree) \
            (sa)->status = sa_ptree_add_tnode_impl((sa), (item)); \
    } while (0)


/**
 *  \brief  Add parse tree non-terminal node
 *
 *  \ref sa_ptree_add_ntnode_impl wrapper.
 *
 *  \param  sa    Syntax analyser
 *  \param  rule  Grammar rule
 */
#define sa_ptree_add_ntnode(sa, rule) \
    do { \
        assert(NULL != (sa)); \
        if ((sa)->ptree) \
            (sa)->status = sa_ptree_add_ntnode_impl((sa), (rule)); \
    } while (0)


/**
 *  \brief  Complete parse tree
 *
 *  \ref sa_ptree_complete_impl wrapper.
 *
 *  \param  sa  Syntax analyser
 */
#define sa_ptree_complete(sa) \
    do { \
        assert(NULL != (sa)); \
        if ((sa)->ptree) \
            (sa)->status = sa_ptree_complete_impl(sa); \
    } while (0)


/*
 * Static functions declarations
 */

static sa_status_t sa_act_on_input(syxa_t *sa);

inline static void sa_ptree_add_node(syxa_t *sa, ptree_node_t *node);

static sa_status_t sa_ptree_add_tnode_impl(syxa_t *sa, const la_item_t *item);

static sa_status_t sa_ptree_add_ntnode_impl(syxa_t *sa, const grammar_rule_t *rule);

static sa_status_t sa_ptree_complete_impl(syxa_t *sa);

static void sa_ptree_cleanup(syxa_t *sa);


/*
 * Syntax analyser interface implementation
 */

syxa_t *sa_create(syxa_t                 *sa,
                  const fsa_t            *la_fsa,
                  size_t                  items_total,
                  const lr1_action_tab_t *action_tab,
                  const lr1_goto_tab_t   *goto_tab,
                  const lr1_rule_tab_t   *rule_tab,
                  const attribute_tab_t  *attr_tab,
                  size_t                  pda_stack_cap_mul,
                  size_t                  pda_stack_limit,
                  int                     reduct_log_enabled,
                  size_t                  reduct_stack_cap_mul,
                  size_t                  reduct_stack_limit,
                  int                     ptree_enabled) {
    assert(NULL != sa);
    assert(NULL != action_tab);
    assert(NULL != goto_tab);

    int stack_status;

    /* Create PDA stack */
    stack_status = sa_stack_create(&sa->state.pda_stack,
                       pda_stack_cap_mul, pda_stack_limit);

    if (stack_status) return NULL;

    /* Initialise PDA stack by initial parser state */
    stack_status = sa_stack_push(&sa->state.pda_stack, 0);

    if (stack_status) {
        sa_stack_destroy(&sa->state.pda_stack);

        return NULL;
    }

    /* Create reduction stack */
    sa->reduct_log = reduct_log_enabled;

    if (sa->reduct_log) {
        stack_status = sa_stack_create(&sa->state.reduct_stack,
                           reduct_stack_cap_mul, reduct_stack_limit);

        if (stack_status) {
            sa_stack_destroy(&sa->state.pda_stack);

            return NULL;
        }
    }

    sa->state.accept = 0;

    /* Initialise parse tree creation */
    sa->ptree = ptree_enabled;
    sa->state.ptree = NULL;

    /* Create lexical analyser */
    la_create(&sa->la, la_fsa, items_total);

    /* Set parser tables */
    sa->action_tab = action_tab;
    sa->goto_tab   = goto_tab;
    sa->rule_tab   = rule_tab;

    /* Set attribute definitions table */
    sa->attr_tab = attr_tab;

    /* Syntax analyser initialised */
    sa->status = SA_OK;

    return sa;
}


void sa_destroy(syxa_t *sa) {
    assert(NULL != sa);

    /* Destroy parse tree (if any) */
    sa_ptree_cleanup(sa);

    /* Destroy lexical analyser */
    la_destroy(&sa->la);

    /* Destroy reduction stack */
    if (sa->reduct_log) sa_stack_destroy(&sa->state.reduct_stack);

    /* Destroy PDA stack */
    sa_stack_destroy(&sa->state.pda_stack);
}


sa_status_t sa_add_data(syxa_t *sa, char *data, size_t size, buffer_cleanup_fn *cleanup_fn, void *user_obj, int is_last) {
    assert(NULL != sa);

    la_status_t la_status = la_add_data(&sa->la, data, size, cleanup_fn, user_obj, is_last);
    if (LA_OK != la_status) {
        return SA_ERROR;
    }

    return SA_OK;
}


sa_status_t sa_parse(syxa_t *sa) {
    sa_status_t status = SA_OK;

    while (!sa_accept(sa)) {
        status = sa_act_on_input(sa);

        if (SA_OK != status) break;
    }

    return status;
}


size_t *sa_derivation(syxa_t *sa, size_t *derivation_len) {
    assert(NULL != sa);
    assert(NULL != derivation_len);
    assert(sa->reduct_log);

    size_t d_len = sa_stack_depth(&sa->state.reduct_stack);

    size_t *d = (size_t *)malloc(d_len * sizeof(size_t));

    if (NULL == d) return NULL;

    size_t i = 0;

    for (; i < d_len; ++i) {
        d[i] = sa_stack_get_top(&sa->state.reduct_stack);

        sa_stack_pop(&sa->state.reduct_stack, 1);
    }

    *derivation_len = d_len;

    return d;
}


ptree_node_t *sa_handover_ptree(syxa_t *sa) {
    assert(NULL != sa);

    ptree_node_t *ptree = sa->state.ptree;
    sa->state.ptree = NULL;

    return ptree;
}


/*
 * Static functions
 */

/**
 *  \brief  Perform one action
 *
 *  The function tries to get next lexical item from the input.
 *  if (one or more) lexical item(s) may be provided,
 *  the function acts on it as defined by the LR parser action table.
 *
 *  After the function is called, either the parser state stays intact
 *  (on input exhaustion) or it changes (as prescribed in the parser
 *  action/goto tables) or parsing shall end (on parse error).
 *
 *  \param  sa  Syntax analyser
 *
 *  \retval SA_OK              if another parsing action was successfully done
 *  \retval SA_INPUT_EXHAUSTED if input is exhausted
 *  \retval SA_PARSE_ERROR     if there is a syntax error in the input
 *  \retval SA_ERROR           on other failure (memory fault etc)
 */
static sa_status_t sa_act_on_input(syxa_t *sa) {
    assert(NULL != sa);

    /* Get lexical item(s) on the input head */
    const la_item_t *items;
    size_t           item_cnt;

    la_status_t la_status = la_get_items(&sa->la, &items, &item_cnt);

    switch (la_status) {
        case LA_OK:
            assert(0 < item_cnt);

            sa->status = SA_OK;

            break;

        case LA_INPUT_EXHAUSTED:
            return sa->status = SA_INPUT_EXHAUSTED;

        case LA_INPUT_INVALID:
            return sa->status = SA_SYNTAX_ERROR;

        case LA_ERROR:
            return sa->status = SA_ERROR;
    }

    /* Get current state */
    size_t state = sa_stack_get_top(&sa->state.pda_stack);

    /* Get action from action table (currect state / input head) */
    const sa_action_t *action    = NULL;
    const la_item_t   *item      = NULL;
    size_t             item_idx  = 0;
    int                item_code;

    assert(item_idx < item_cnt);

    do {
        item = items + item_idx++;

        item_code = la_item_code(item);

        action = sa_action_table_at(sa->action_tab, state, item_code);

    } while (NULL == action && item_idx < item_cnt);

    /* Perform selected action */
    switch (sa_action_type(action)) {
        case SA_ACTION_SHIFT: {
            /* Next state is defined in the action */
            size_t next_state = sa_action_shift_next_state(action);

            sa_stack_push(&sa->state.pda_stack, next_state);

            /* Add another parse tree terminal leaf */
            sa_ptree_add_tnode(sa, item);

            /* Consume (aka shift) lexical item on the input head */
            la_read_item(&sa->la, item_code);

            break;
        }

        case SA_ACTION_REDUCE: {
            int stack_status;

            /* Get reduce rule A => X */
            size_t rule_no = sa_action_reduce_rule_no(action);
            const grammar_rule_t *rule = sa_rule_table_at(sa->rule_tab, rule_no);

            /* Remember reduction path */
            if (sa->reduct_log) {
                stack_status = sa_stack_push(&sa->state.reduct_stack, rule_no);

                switch (stack_status) {
                    case 0:
                        /* OK */
                        break;

                    case EOVERFLOW:
                        /* Reduction too long */

                        /* Fall-through to the default branch is intentional */

                    default:
                        /* Something sinister (memory fault or so) */
                        return sa->status = SA_ERROR;
                }
            }

            /* Pop |X| states from stack (plus current top) */
            size_t rhs_sym_cnt = grammar_rule_rhs_symbol_count(rule);

            sa_stack_pop(&sa->state.pda_stack, rhs_sym_cnt);

            /* Next state is defined in goto table (by current stack top and NT A) */
            state = sa_stack_get_top(&sa->state.pda_stack);

            int    lhs_nt     = grammar_rule_lhs_non_terminal(rule);
            size_t next_state = sa_goto_table_at(sa->goto_tab, state, lhs_nt);

            assert(SA_NULL_TARGET != next_state);

            stack_status = sa_stack_push(&sa->state.pda_stack, next_state);

            switch (stack_status) {
                case 0:
                    /* OK */
                    break;

                case EOVERFLOW:
                    /*
                     * Stack too deep
                     *
                     * This almost definitely means deep grammar recursion;
                     * either the input is very wierd or someone is deliberately
                     * playing dirty with us...
                     * Anyway, we shall stop it right now.
                     */
                    sa->status = SA_ERROR;

                    break;

                default:
                    /* Something sinister (memory fault or so) */
                    sa->status = SA_ERROR;
            }

            /* Add non-terminal node to parse tree */
            sa_ptree_add_ntnode(sa, rule);

            break;
        }

        case SA_ACTION_ACCEPT:
            /*
             * Sanity checks
             *
             * Accept action is de-facto augmented grammar
             * root reduce action on EOF (with no next state).
             */
            assert(LEXIG_EOF == item_code);

            sa_stack_pop(&sa->state.pda_stack, 1);

            assert(1 == sa_stack_depth(&sa->state.pda_stack));
            assert(0 == sa_stack_get_top(&sa->state.pda_stack));

            sa->state.accept = 1;

            /* Parse tree finishing touches */
            sa_ptree_complete(sa);

            break;

        case SA_ACTION_REJECT:
            sa->status = SA_SYNTAX_ERROR;

            break;
    }

    return sa->status;
}


/**
 *  \brief  Add parse tree node (as another top-level sibling)
 *
 *  \param  sa    Syntax analyser
 *  \param  node  Parse tree node
 */
inline static void sa_ptree_add_node(syxa_t *sa, ptree_node_t *node) {
    assert(NULL != sa);

    if (NULL != sa->state.ptree) {
        ptree_node_set_prev(node, sa->state.ptree);
        ptree_node_set_next(sa->state.ptree, node);
    }

    sa->state.ptree = node;
}


/**
 *  \brief  Add parse tree leaf terminal node
 *
 *  \param  sa    Syntax analyser
 *  \param  item  Lexical item
 *
 *  \retval SA_OK     on success
 *  \retval SA_ERROR  on other failure (memory fault etc)
 */
static sa_status_t sa_ptree_add_tnode_impl(syxa_t *sa, const la_item_t *item) {
    assert(NULL != sa);

    int item_code = la_item_code(item);

    const attr_classes_t *attr_classes =
        (*sa->attr_tab->tsyms_classes) + item_code - 1;

    const attr_eval_descr_refs_t *attr_evals =
        (*sa->attr_tab->tsyms_expl_evals)[item_code - 1];

    const attr_name_fsa_t *attr_resolver =
        sa_attr_table_tsym_name_fsa(sa->attr_tab, item_code);

    ptree_node_t *node = ptree_tnode_create(
        item,
        attr_classes->class_cnt,
        attr_classes->classes,
        attr_evals,
        NULL,
        attr_resolver);

    if (NULL == node) return SA_ERROR;

    sa_ptree_add_node(sa, node);

    /* Resolve symbol attributes dependencies (it only has aggregated attrs) */
    if (!ptree_resolve_attr_dependencies(node, ATTR_TYPE_AGGREGATED))
        return SA_ERROR;

    return SA_OK;
}


/**
 *  \brief  Add parse tree non-terminal node
 *
 *  Note that this typically mean an inner node;
 *  however, if the terminal was derived using an epsilon rule,
 *  it may be a leaf, too.
 *  Although such non-terminal leaves don't represent any code,
 *  they must be included in the tree for 2 reasons:
 *  1/ not including them would break/complicate the tree building
 *     mechanism (which copies LR parsing process)
 *  2/ leave non-terminal nodes may still be used for attributes
 *     aggregation (although such aggreg. attribute computation
 *     is a nular function since there are no RHS symbols)
 *
 *  \param  sa    Syntax analyser
 *  \param  rule  Grammar rule
 *
 *  \retval SA_OK     on success
 *  \retval SA_ERROR  on other failure (memory fault etc)
 */
static sa_status_t sa_ptree_add_ntnode_impl(syxa_t *sa, const grammar_rule_t *rule) {
    assert(NULL != sa);

    int lhs_nt = grammar_rule_lhs_non_terminal(rule);

    const attr_classes_t *attr_classes =
        (*sa->attr_tab->ntsyms_classes) + lhs_nt;

    const attr_eval_descr_refs_t *g_attr_evals =
        (*sa->attr_tab->ntsyms_expl_evals)[lhs_nt];

    const attr_eval_descr_refs_t *s_attr_evals = rule->lhs_attr_evals;

    const attr_name_fsa_t *attr_resolver =
        sa_attr_table_ntsym_name_fsa(sa->attr_tab, lhs_nt);

    /* Create new node */
    ptree_node_t *node = ptree_ntnode_create(
        rule,
        attr_classes->class_cnt,
        attr_classes->classes,
        g_attr_evals,
        s_attr_evals,
        attr_resolver);

    if (NULL == node) return SA_ERROR;

    sa_status_t status = SA_OK;

    /* Get the node children count (i.e. rule RHS symbol count) */
    size_t child_cnt = grammar_rule_rhs_symbol_count(rule);

    /* Set node children (one per each rule RHS symbol) */
    if (child_cnt) {
        ptree_node_t *child, *last_child;

        child = last_child = sa->state.ptree;

        size_t child_idx = child_cnt;

        for (;;) {
            --child_idx;

            assert(NULL != child);

            ptree_node_set_parent(child, node);

            /* Set child's inherited attribute evalators */
            const attr_eval_descr_refs_t *attr_evals =
                rule->rhs_attr_evals[child_idx];

            if (!ptree_node_attr_evaluators(child, attr_evals))
                status = SA_ERROR;

            if (0 == child_idx) break;

            child = ptree_node_get_prev(child);
        }

        sa->state.ptree = ptree_node_get_prev(child);

        ptree_node_set_prev(child, last_child);  /* see ptree.h doc */
        ptree_node_set_child(node, child, child_cnt);
    }

    sa_ptree_add_node(sa, node);  /* also sets sa->state.ptree->next */

    /* Resolve node aggregated attributes dependencies */
    if (SA_OK == status)
        if (!ptree_resolve_attr_dependencies(node, ATTR_TYPE_AGGREGATED))
            status = SA_ERROR;

    /* Resolve node children inherited attgributes dependencies */
    if (SA_OK == status)
        if (!ptree_resolve_attr_dependencies(node, ATTR_TYPE_INHERITED))
            status = SA_ERROR;

    return status;
}


/**
 *  \brief  Complete parse tree
 *
 *  \param  sa  Syntax analyser
 *
 *  \retval SA_OK     on success
 *  \retval SA_ERROR  on other failure (memory fault etc)
 */
static sa_status_t sa_ptree_complete_impl(syxa_t *sa) {
    assert(NULL != sa);

    /* Check root node validity */
    assert(NULL != sa->state.ptree);
    assert(NULL == ptree_node_get_prev(sa->state.ptree));
    assert(NULL == ptree_node_get_next(sa->state.ptree));

    /* See ptree.h doc for info on why this is done */
    ptree_node_set_prev(sa->state.ptree, sa->state.ptree);

    return SA_OK;
}


/**
 *  \brief  Cleanup (even incomplete) parse tree
 *
 *  The function calls \ref ptree_destroy on all parse sub-trees
 *  in the analyser's parse tree list
 *  (there may be more than one since the parse tree is created
 *  bottom-to-top, i.e. the LR way).
 *
 *  \param  sa  Syntax analyser
 */
static void sa_ptree_cleanup(syxa_t *sa) {
    assert(NULL != sa);

    ptree_node_t *node, *last_node;

    node = last_node = sa->state.ptree;

    while (NULL != node) {
        ptree_node_set_next(node, NULL);

        ptree_node_t *root = node;
        node = ptree_node_get_prev(root);

        ptree_destroy(root);

        if (node == last_node) break;
    }
}
