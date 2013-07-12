/**
 *  \brief  LR(1) parser tables definitions
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2012/08/24
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

#include "test.nonterminals.h"
#include "test.extern_attr.h"
#include "builtin_attr.h"
#include "grammar.h"
#include "syntax_analyser.h"

#include <assert.h>


/*
 * Actions
 */

static const sa_action_t s3  = { .type = SA_ACTION_SHIFT, .t.shift.next_state = 3 };
static const sa_action_t s4  = { .type = SA_ACTION_SHIFT, .t.shift.next_state = 4 };
static const sa_action_t s6  = { .type = SA_ACTION_SHIFT, .t.shift.next_state = 6 };
static const sa_action_t s7  = { .type = SA_ACTION_SHIFT, .t.shift.next_state = 7 };
static const sa_action_t s11 = { .type = SA_ACTION_SHIFT, .t.shift.next_state = 11 };

static const sa_action_t r1 = { .type = SA_ACTION_REDUCE, .t.reduce.rule_no = 1 };
static const sa_action_t r2 = { .type = SA_ACTION_REDUCE, .t.reduce.rule_no = 2 };
static const sa_action_t r3 = { .type = SA_ACTION_REDUCE, .t.reduce.rule_no = 3 };
static const sa_action_t r4 = { .type = SA_ACTION_REDUCE, .t.reduce.rule_no = 4 };
static const sa_action_t r5 = { .type = SA_ACTION_REDUCE, .t.reduce.rule_no = 5 };
static const sa_action_t r6 = { .type = SA_ACTION_REDUCE, .t.reduce.rule_no = 6 };

static const sa_action_t acc = { .type = SA_ACTION_ACCEPT };


/** Action table implementation */
static const sa_action_t *action_tab_impl[12 * 6] = {
    /*  0:EOF    */  NULL,
    /*  0:number */  &s3,
    /*  0:add    */  NULL,
    /*  0:mul    */  NULL,
    /*  0:rpar   */  NULL,
    /*  0:lpar   */  &s4,

    /*  1:EOF    */  &acc,
    /*  1:number */  NULL,
    /*  1:add    */  &s6,
    /*  1:mul    */  NULL,
    /*  1:rpar   */  NULL,
    /*  1:lpar   */  NULL,

    /*  2:EOF    */  &r2,
    /*  2:number */  NULL,
    /*  2:add    */  &r2,
    /*  2:mul    */  &s7,
    /*  2:rpar   */  &r2,
    /*  2:lpar   */  NULL,

    /*  3:EOF    */  &r6,
    /*  3:number */  NULL,
    /*  3:add    */  &r6,
    /*  3:mul    */  &r6,
    /*  3:rpar   */  &r6,
    /*  3:lpar   */  NULL,

    /*  4:EOF    */  NULL,
    /*  4:number */  &s3,
    /*  4:add    */  NULL,
    /*  4:mul    */  NULL,
    /*  4:rpar   */  NULL,
    /*  4:lpar   */  &s4,

    /*  5:EOF    */  &r4,
    /*  5:number */  NULL,
    /*  5:add    */  &r4,
    /*  5:mul    */  &r4,
    /*  5:rpar   */  &r4,
    /*  5:lpar   */  NULL,

    /*  6:EOF    */  NULL,
    /*  6:number */  &s3,
    /*  6:add    */  NULL,
    /*  6:mul    */  NULL,
    /*  6:rpar   */  NULL,
    /*  6:lpar   */  &s4,

    /*  7:EOF    */  NULL,
    /*  7:number */  &s3,
    /*  7:add    */  NULL,
    /*  7:mul    */  NULL,
    /*  7:rpar   */  NULL,
    /*  7:lpar   */  &s4,

    /*  8:EOF    */  NULL,
    /*  8:number */  NULL,
    /*  8:add    */  &s6,
    /*  8:mul    */  NULL,
    /*  8:rpar   */  &s11,
    /*  8:lpar   */  NULL,

    /*  9:EOF    */  &r1,
    /*  9:number */  NULL,
    /*  9:add    */  &r1,
    /*  9:mul    */  &s7,
    /*  9:rpar   */  &r1,
    /*  9:lpar   */  NULL,

    /* 10:EOF    */  &r3,
    /* 10:number */  NULL,
    /* 10:add    */  &r3,
    /* 10:mul    */  &r3,
    /* 10:rpar   */  &r3,
    /* 10:lpar   */  NULL,

    /* 11:EOF    */  &r5,
    /* 11:number */  NULL,
    /* 11:add    */  &r5,
    /* 11:mul    */  &r5,
    /* 11:rpar   */  &r5,
    /* 11:lpar   */  NULL,
};

/* Action table */
const lr1_action_tab_t test_action_tab = {
    .state_cnt    = 12,
    .lex_item_cnt = 6,
    .impl         = &action_tab_impl,
};


/** Goto table implementation */
static const size_t goto_tab_impl[12 * 3] = {
    /*  0:F */  2,
    /*  0:T */  5,
    /*  0:E */  1,

    /*  1:F */  SA_NULL_TARGET,
    /*  1:T */  SA_NULL_TARGET,
    /*  1:E */  SA_NULL_TARGET,

    /*  2:F */  SA_NULL_TARGET,
    /*  2:T */  SA_NULL_TARGET,
    /*  2:E */  SA_NULL_TARGET,

    /*  3:F */  SA_NULL_TARGET,
    /*  3:T */  SA_NULL_TARGET,
    /*  3:E */  SA_NULL_TARGET,

    /*  4:F */  2,
    /*  4:T */  5,
    /*  4:E */  8,

    /*  5:F */  SA_NULL_TARGET,
    /*  5:T */  SA_NULL_TARGET,
    /*  5:E */  SA_NULL_TARGET,

    /*  6:F */  9,
    /*  6:T */  5,
    /*  6:E */  SA_NULL_TARGET,

    /*  7:F */  SA_NULL_TARGET,
    /*  7:T */  10,
    /*  7:E */  SA_NULL_TARGET,

    /*  8:F */  SA_NULL_TARGET,
    /*  8:T */  SA_NULL_TARGET,
    /*  8:E */  SA_NULL_TARGET,

    /*  9:F */  SA_NULL_TARGET,
    /*  9:T */  SA_NULL_TARGET,
    /*  9:E */  SA_NULL_TARGET,

    /* 10:F */  SA_NULL_TARGET,
    /* 10:T */  SA_NULL_TARGET,
    /* 10:E */  SA_NULL_TARGET,

    /* 11:F */  SA_NULL_TARGET,
    /* 11:T */  SA_NULL_TARGET,
    /* 11:E */  SA_NULL_TARGET,
};

/* Goto table */
const lr1_goto_tab_t test_goto_tab = {
    .state_cnt    = 12,
    .non_term_cnt = 3,
    .impl         = &goto_tab_impl,
};


/*
 * Attribute evaluators wrappers prototypes
 */

static attr_evaluator_t external__mul__arity2;
static attr_evaluator_t external__sum__arity2;
static attr_evaluator_t builtin__get_token__arity0;
static attr_evaluator_t external__token2num__arity1;
static attr_evaluator_t builtin__reference__arity1;


/*
 * Attribute destructors wrappers prototypes
 */

static attr_destructor_t external__destroy_value__arity1;


/*
 * Terminal symbols attribute class descriptors
 */

/** Grammar symbol number attribute class descriptors */
static const attr_class_descr_t sym_number_attr_class_descrs[2] = {
    /* value */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "value",
        .destroy = external__destroy_value__arity1,
    },
    /* token */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "token",
        .destroy = NULL,
    },
};

/** Grammar symbol add attribute class descriptors */
static const attr_class_descr_t sym_add_attr_class_descrs[1] = {
    /* token */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "token",
        .destroy = NULL,
    },
};

/** Grammar symbol mul attribute class descriptors */
static const attr_class_descr_t sym_mul_attr_class_descrs[1] = {
    /* token */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "token",
        .destroy = NULL,
    },
};

/** Grammar symbol rpar attribute class descriptors */
static const attr_class_descr_t sym_rpar_attr_class_descrs[1] = {
    /* token */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "token",
        .destroy = NULL,
    },
};

/** Grammar symbol lpar attribute class descriptors */
static const attr_class_descr_t sym_lpar_attr_class_descrs[1] = {
    /* token */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "token",
        .destroy = NULL,
    },
};


/**
 *  \brief  Terminal symbols attribute classes
 *
 *  Indexation follows lexical symbols \c LEXI_* definitions
 *  with the exception that the lexical item values begin
 *  with 1 (0 being reserved for the EoF pseudo-lexical item).
 *
 *  In other words, \c tsyms_attr_classes[LEXI_something \c - \c 1]
 *  provides terminal symbol \c something attribute classes descriptors.
 */
static const attr_classes_t tsyms_attr_classes[5] = {
    /* number */  {
        .class_cnt = 2,
        .classes   = &sym_number_attr_class_descrs,
    },
    /* add */  {
        .class_cnt = 1,
        .classes   = &sym_add_attr_class_descrs,
    },
    /* mul */  {
        .class_cnt = 1,
        .classes   = &sym_mul_attr_class_descrs,
    },
    /* rpar */  {
        .class_cnt = 1,
        .classes   = &sym_rpar_attr_class_descrs,
    },
    /* lpar */  {
        .class_cnt = 1,
        .classes   = &sym_lpar_attr_class_descrs,
    },
};


/*
 * Non-terminal symbols attribute class descriptors
 */

/** Grammar symbol F attribute class descriptors */
static const attr_class_descr_t sym_F_attr_class_descrs[1] = {
    /* value */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "value",
        .destroy = external__destroy_value__arity1,
    },
};

/** Grammar symbol T attribute class descriptors */
static const attr_class_descr_t sym_T_attr_class_descrs[1] = {
    /* value */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "value",
        .destroy = external__destroy_value__arity1,
    },
};

/** Grammar symbol E attribute class descriptors */
static const attr_class_descr_t sym_E_attr_class_descrs[1] = {
    /* value */  {
        .type    = ATTR_TYPE_AGGREGATED,
        .id      = "value",
        .destroy = external__destroy_value__arity1,
    },
};


/**
 *  \brief  Non-terminal symbols attribute classes
 *
 *  Indexation follows non-terminals \c NT_* definitions.
 *
 *  In other words, \c ntsyms_attr_classes[NT_SOMETHING]
 *  provides non-terminal symbol \c SOMETHING attribute classes descriptors.
 */
static const attr_classes_t ntsyms_attr_classes[3] = {
    /* F */  {
        .class_cnt = 1,
        .classes   = &sym_F_attr_class_descrs,
    },
    /* T */  {
        .class_cnt = 1,
        .classes   = &sym_T_attr_class_descrs,
    },
    /* E */  {
        .class_cnt = 1,
        .classes   = &sym_E_attr_class_descrs,
    },
};


/*
 * Terminal symbols attribute evaluators
 */

/** Grammar symbol number attribute value evaluator descriptor */
static const attr_eval_descr_t sym_number_attr_value_eval_descr = {
    .eval       = &external__token2num__arity1,
    .dep_cnt    = 1,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           0,          1 },
    }
};

/** Grammar symbol number attribute token evaluator descriptor */
static const attr_eval_descr_t sym_number_attr_token_eval_descr = {
    .eval       = &builtin__get_token__arity0,
    .dep_cnt    = 0,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
    }
};

/** Grammar symbol add attribute token evaluator descriptor */
static const attr_eval_descr_t sym_add_attr_token_eval_descr = {
    .eval       = &builtin__get_token__arity0,
    .dep_cnt    = 0,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
    }
};

/** Grammar symbol mul attribute token evaluator descriptor */
static const attr_eval_descr_t sym_mul_attr_token_eval_descr = {
    .eval       = &builtin__get_token__arity0,
    .dep_cnt    = 0,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
    }
};

/** Grammar symbol rpar attribute token evaluator descriptor */
static const attr_eval_descr_t sym_rpar_attr_token_eval_descr = {
    .eval       = &builtin__get_token__arity0,
    .dep_cnt    = 0,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
    }
};

/** Grammar symbol lpar attribute token evaluator descriptor */
static const attr_eval_descr_t sym_lpar_attr_token_eval_descr = {
    .eval       = &builtin__get_token__arity0,
    .dep_cnt    = 0,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
    }
};

/** Grammar symbol number explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_number_attr_eval_descrs = {
    /* value */  &sym_number_attr_value_eval_descr,
    /* token */  &sym_number_attr_token_eval_descr,
};

/** Grammar symbol add explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_add_attr_eval_descrs = {
    /* token */  &sym_add_attr_token_eval_descr,
};

/** Grammar symbol mul explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_mul_attr_eval_descrs = {
    /* token */  &sym_mul_attr_token_eval_descr,
};

/** Grammar symbol rpar explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_rpar_attr_eval_descrs = {
    /* token */  &sym_rpar_attr_token_eval_descr,
};

/** Grammar symbol lpar explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_lpar_attr_eval_descrs = {
    /* token */  &sym_lpar_attr_token_eval_descr,
};

/** Explicit attribute evaluators */
static const attr_eval_descr_refs_t *tsyms_attr_eval_descrs[5] = {
    /* number */  &sym_number_attr_eval_descrs,
    /*    add */  &sym_add_attr_eval_descrs,
    /*    mul */  &sym_mul_attr_eval_descrs,
    /*   rpar */  &sym_rpar_attr_eval_descrs,
    /*   lpar */  &sym_lpar_attr_eval_descrs,
};


/*
 * Non-terminal symbols attribute evaluators
 */

/** Grammar symbol F attribute value evaluator descriptor (for rule #3) */
static const attr_eval_descr_t sym_F_idx0_attr_value_rule3_eval_descr = {
    .eval       = &external__mul__arity2,
    .dep_cnt    = 2,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           1,          0 },
        {           3,          0 },
    }
};

/** Grammar symbol F attribute value evaluator descriptor (for rule #4) */
static const attr_eval_descr_t sym_F_idx0_attr_value_rule4_eval_descr = {
    .eval       = &builtin__reference__arity1,
    .dep_cnt    = 1,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           1,          0 },
    }
};

/** Grammar symbol T attribute value evaluator descriptor (for rule #5) */
static const attr_eval_descr_t sym_T_idx0_attr_value_rule5_eval_descr = {
    .eval       = &builtin__reference__arity1,
    .dep_cnt    = 1,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           2,          0 },
    }
};

/** Grammar symbol T attribute value evaluator descriptor (for rule #6) */
static const attr_eval_descr_t sym_T_idx0_attr_value_rule6_eval_descr = {
    .eval       = &builtin__reference__arity1,
    .dep_cnt    = 1,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           1,          0 },
    }
};

/** Grammar symbol E attribute value evaluator descriptor (for rule #1) */
static const attr_eval_descr_t sym_E_idx0_attr_value_rule1_eval_descr = {
    .eval       = &external__sum__arity2,
    .dep_cnt    = 2,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           1,          0 },
        {           3,          0 },
    }
};

/** Grammar symbol E attribute value evaluator descriptor (for rule #2) */
static const attr_eval_descr_t sym_E_idx0_attr_value_rule2_eval_descr = {
    .eval       = &builtin__reference__arity1,
    .dep_cnt    = 1,
    .dep_descrs = {
        /* Symbol idx   Attr. idx */
        {           1,          0 },
    }
};

/** Grammar symbol F explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_F_attr_eval_descrs = {
    /* value */  NULL,
};

/** Grammar symbol T explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_T_attr_eval_descrs = {
    /* value */  NULL,
};

/** Grammar symbol E explicit attribute evaluators */
static const attr_eval_descr_refs_t sym_E_attr_eval_descrs = {
    /* value */  NULL,
};

/** Explicit attribute evaluators */
static const attr_eval_descr_refs_t *ntsyms_attr_eval_descrs[3] = {
    /* F */  &sym_F_attr_eval_descrs,
    /* T */  &sym_T_attr_eval_descrs,
    /* E */  &sym_E_attr_eval_descrs,
};


/*
 * Terminal symbols attribute names resolution FSA
 */

/** Symbol number attribute token resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_token_leaf = {
    .locator = {
        .index = 1,
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_token_branches[1] = {
    /* token (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_number_attr_name_fsa_token_leaf
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_token = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_token_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_toke_branches[1] = {
    /* token prefix */  {
        .character = 'n',
        .sub_fsa   = &sym_number_attr_name_fsa_token
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_toke = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_toke_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_tok_branches[1] = {
    /* toke prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_number_attr_name_fsa_toke
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_tok = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_tok_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_to_branches[1] = {
    /* tok prefix */  {
        .character = 'k',
        .sub_fsa   = &sym_number_attr_name_fsa_tok
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_to = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_to_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_t_branches[1] = {
    /* to prefix */  {
        .character = 'o',
        .sub_fsa   = &sym_number_attr_name_fsa_to
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_t = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_t_branches
    }
};

/** Symbol number attribute value resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_value_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_value_branches[1] = {
    /* value (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_number_attr_name_fsa_value_leaf
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_value = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_value_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_valu_branches[1] = {
    /* value prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_number_attr_name_fsa_value
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_valu = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_valu_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_val_branches[1] = {
    /* valu prefix */  {
        .character = 'u',
        .sub_fsa   = &sym_number_attr_name_fsa_valu
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_val = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_val_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_va_branches[1] = {
    /* val prefix */  {
        .character = 'l',
        .sub_fsa   = &sym_number_attr_name_fsa_val
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_va = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_va_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa_v_branches[1] = {
    /* va prefix */  {
        .character = 'a',
        .sub_fsa   = &sym_number_attr_name_fsa_va
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_v = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_number_attr_name_fsa_v_branches
    }
};

/** Symbol number attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_number_attr_name_fsa__branches[2] = {
    /* t prefix */  {
        .character = 't',
        .sub_fsa   = &sym_number_attr_name_fsa_t
    },
    /* v prefix */  {
        .character = 'v',
        .sub_fsa   = &sym_number_attr_name_fsa_v
    },
};

/** Symbol number attribute names resolution FSA */
static const attr_name_fsa_t sym_number_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 2,
        .branches   = &sym_number_attr_name_fsa__branches
    }
};

/** Symbol add attribute token resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_token_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol add attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_add_attr_name_fsa_token_branches[1] = {
    /* token (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_add_attr_name_fsa_token_leaf
    },
};

/** Symbol add attribute names resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_token = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_add_attr_name_fsa_token_branches
    }
};

/** Symbol add attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_add_attr_name_fsa_toke_branches[1] = {
    /* token prefix */  {
        .character = 'n',
        .sub_fsa   = &sym_add_attr_name_fsa_token
    },
};

/** Symbol add attribute names resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_toke = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_add_attr_name_fsa_toke_branches
    }
};

/** Symbol add attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_add_attr_name_fsa_tok_branches[1] = {
    /* toke prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_add_attr_name_fsa_toke
    },
};

/** Symbol add attribute names resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_tok = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_add_attr_name_fsa_tok_branches
    }
};

/** Symbol add attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_add_attr_name_fsa_to_branches[1] = {
    /* tok prefix */  {
        .character = 'k',
        .sub_fsa   = &sym_add_attr_name_fsa_tok
    },
};

/** Symbol add attribute names resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_to = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_add_attr_name_fsa_to_branches
    }
};

/** Symbol add attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_add_attr_name_fsa_t_branches[1] = {
    /* to prefix */  {
        .character = 'o',
        .sub_fsa   = &sym_add_attr_name_fsa_to
    },
};

/** Symbol add attribute names resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_t = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_add_attr_name_fsa_t_branches
    }
};

/** Symbol add attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_add_attr_name_fsa__branches[1] = {
    /* t prefix */  {
        .character = 't',
        .sub_fsa   = &sym_add_attr_name_fsa_t
    },
};

/** Symbol add attribute names resolution FSA */
static const attr_name_fsa_t sym_add_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_add_attr_name_fsa__branches
    }
};

/** Symbol mul attribute token resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_token_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol mul attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_mul_attr_name_fsa_token_branches[1] = {
    /* token (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_mul_attr_name_fsa_token_leaf
    },
};

/** Symbol mul attribute names resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_token = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_mul_attr_name_fsa_token_branches
    }
};

/** Symbol mul attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_mul_attr_name_fsa_toke_branches[1] = {
    /* token prefix */  {
        .character = 'n',
        .sub_fsa   = &sym_mul_attr_name_fsa_token
    },
};

/** Symbol mul attribute names resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_toke = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_mul_attr_name_fsa_toke_branches
    }
};

/** Symbol mul attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_mul_attr_name_fsa_tok_branches[1] = {
    /* toke prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_mul_attr_name_fsa_toke
    },
};

/** Symbol mul attribute names resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_tok = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_mul_attr_name_fsa_tok_branches
    }
};

/** Symbol mul attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_mul_attr_name_fsa_to_branches[1] = {
    /* tok prefix */  {
        .character = 'k',
        .sub_fsa   = &sym_mul_attr_name_fsa_tok
    },
};

/** Symbol mul attribute names resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_to = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_mul_attr_name_fsa_to_branches
    }
};

/** Symbol mul attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_mul_attr_name_fsa_t_branches[1] = {
    /* to prefix */  {
        .character = 'o',
        .sub_fsa   = &sym_mul_attr_name_fsa_to
    },
};

/** Symbol mul attribute names resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_t = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_mul_attr_name_fsa_t_branches
    }
};

/** Symbol mul attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_mul_attr_name_fsa__branches[1] = {
    /* t prefix */  {
        .character = 't',
        .sub_fsa   = &sym_mul_attr_name_fsa_t
    },
};

/** Symbol mul attribute names resolution FSA */
static const attr_name_fsa_t sym_mul_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_mul_attr_name_fsa__branches
    }
};

/** Symbol rpar attribute token resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_token_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol rpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_rpar_attr_name_fsa_token_branches[1] = {
    /* token (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_rpar_attr_name_fsa_token_leaf
    },
};

/** Symbol rpar attribute names resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_token = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_rpar_attr_name_fsa_token_branches
    }
};

/** Symbol rpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_rpar_attr_name_fsa_toke_branches[1] = {
    /* token prefix */  {
        .character = 'n',
        .sub_fsa   = &sym_rpar_attr_name_fsa_token
    },
};

/** Symbol rpar attribute names resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_toke = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_rpar_attr_name_fsa_toke_branches
    }
};

/** Symbol rpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_rpar_attr_name_fsa_tok_branches[1] = {
    /* toke prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_rpar_attr_name_fsa_toke
    },
};

/** Symbol rpar attribute names resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_tok = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_rpar_attr_name_fsa_tok_branches
    }
};

/** Symbol rpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_rpar_attr_name_fsa_to_branches[1] = {
    /* tok prefix */  {
        .character = 'k',
        .sub_fsa   = &sym_rpar_attr_name_fsa_tok
    },
};

/** Symbol rpar attribute names resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_to = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_rpar_attr_name_fsa_to_branches
    }
};

/** Symbol rpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_rpar_attr_name_fsa_t_branches[1] = {
    /* to prefix */  {
        .character = 'o',
        .sub_fsa   = &sym_rpar_attr_name_fsa_to
    },
};

/** Symbol rpar attribute names resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_t = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_rpar_attr_name_fsa_t_branches
    }
};

/** Symbol rpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_rpar_attr_name_fsa__branches[1] = {
    /* t prefix */  {
        .character = 't',
        .sub_fsa   = &sym_rpar_attr_name_fsa_t
    },
};

/** Symbol rpar attribute names resolution FSA */
static const attr_name_fsa_t sym_rpar_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_rpar_attr_name_fsa__branches
    }
};

/** Symbol lpar attribute token resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_token_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol lpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_lpar_attr_name_fsa_token_branches[1] = {
    /* token (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_lpar_attr_name_fsa_token_leaf
    },
};

/** Symbol lpar attribute names resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_token = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_lpar_attr_name_fsa_token_branches
    }
};

/** Symbol lpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_lpar_attr_name_fsa_toke_branches[1] = {
    /* token prefix */  {
        .character = 'n',
        .sub_fsa   = &sym_lpar_attr_name_fsa_token
    },
};

/** Symbol lpar attribute names resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_toke = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_lpar_attr_name_fsa_toke_branches
    }
};

/** Symbol lpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_lpar_attr_name_fsa_tok_branches[1] = {
    /* toke prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_lpar_attr_name_fsa_toke
    },
};

/** Symbol lpar attribute names resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_tok = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_lpar_attr_name_fsa_tok_branches
    }
};

/** Symbol lpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_lpar_attr_name_fsa_to_branches[1] = {
    /* tok prefix */  {
        .character = 'k',
        .sub_fsa   = &sym_lpar_attr_name_fsa_tok
    },
};

/** Symbol lpar attribute names resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_to = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_lpar_attr_name_fsa_to_branches
    }
};

/** Symbol lpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_lpar_attr_name_fsa_t_branches[1] = {
    /* to prefix */  {
        .character = 'o',
        .sub_fsa   = &sym_lpar_attr_name_fsa_to
    },
};

/** Symbol lpar attribute names resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_t = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_lpar_attr_name_fsa_t_branches
    }
};

/** Symbol lpar attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_lpar_attr_name_fsa__branches[1] = {
    /* t prefix */  {
        .character = 't',
        .sub_fsa   = &sym_lpar_attr_name_fsa_t
    },
};

/** Symbol lpar attribute names resolution FSA */
static const attr_name_fsa_t sym_lpar_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_lpar_attr_name_fsa__branches
    }
};

/** Terminals attr. names resolution FSA list */
static const attr_name_fsa_t *tsyms_attr_names_fsa[5] = {
    /* number */  &sym_number_attr_name_fsa_,
    /*    add */  &sym_add_attr_name_fsa_,
    /*    mul */  &sym_mul_attr_name_fsa_,
    /*   rpar */  &sym_rpar_attr_name_fsa_,
    /*   lpar */  &sym_lpar_attr_name_fsa_,
};

/*
 * Non-terminal symbols attribute names resolution FSA
 */

/** Symbol F attribute value resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_value_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol F attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_F_attr_name_fsa_value_branches[1] = {
    /* value (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_F_attr_name_fsa_value_leaf
    },
};

/** Symbol F attribute names resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_value = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_F_attr_name_fsa_value_branches
    }
};

/** Symbol F attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_F_attr_name_fsa_valu_branches[1] = {
    /* value prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_F_attr_name_fsa_value
    },
};

/** Symbol F attribute names resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_valu = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_F_attr_name_fsa_valu_branches
    }
};

/** Symbol F attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_F_attr_name_fsa_val_branches[1] = {
    /* valu prefix */  {
        .character = 'u',
        .sub_fsa   = &sym_F_attr_name_fsa_valu
    },
};

/** Symbol F attribute names resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_val = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_F_attr_name_fsa_val_branches
    }
};

/** Symbol F attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_F_attr_name_fsa_va_branches[1] = {
    /* val prefix */  {
        .character = 'l',
        .sub_fsa   = &sym_F_attr_name_fsa_val
    },
};

/** Symbol F attribute names resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_va = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_F_attr_name_fsa_va_branches
    }
};

/** Symbol F attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_F_attr_name_fsa_v_branches[1] = {
    /* va prefix */  {
        .character = 'a',
        .sub_fsa   = &sym_F_attr_name_fsa_va
    },
};

/** Symbol F attribute names resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_v = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_F_attr_name_fsa_v_branches
    }
};

/** Symbol F attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_F_attr_name_fsa__branches[1] = {
    /* v prefix */  {
        .character = 'v',
        .sub_fsa   = &sym_F_attr_name_fsa_v
    },
};

/** Symbol F attribute names resolution FSA */
static const attr_name_fsa_t sym_F_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_F_attr_name_fsa__branches
    }
};

/** Symbol T attribute value resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_value_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol T attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_T_attr_name_fsa_value_branches[1] = {
    /* value (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_T_attr_name_fsa_value_leaf
    },
};

/** Symbol T attribute names resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_value = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_T_attr_name_fsa_value_branches
    }
};

/** Symbol T attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_T_attr_name_fsa_valu_branches[1] = {
    /* value prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_T_attr_name_fsa_value
    },
};

/** Symbol T attribute names resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_valu = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_T_attr_name_fsa_valu_branches
    }
};

/** Symbol T attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_T_attr_name_fsa_val_branches[1] = {
    /* valu prefix */  {
        .character = 'u',
        .sub_fsa   = &sym_T_attr_name_fsa_valu
    },
};

/** Symbol T attribute names resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_val = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_T_attr_name_fsa_val_branches
    }
};

/** Symbol T attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_T_attr_name_fsa_va_branches[1] = {
    /* val prefix */  {
        .character = 'l',
        .sub_fsa   = &sym_T_attr_name_fsa_val
    },
};

/** Symbol T attribute names resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_va = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_T_attr_name_fsa_va_branches
    }
};

/** Symbol T attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_T_attr_name_fsa_v_branches[1] = {
    /* va prefix */  {
        .character = 'a',
        .sub_fsa   = &sym_T_attr_name_fsa_va
    },
};

/** Symbol T attribute names resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_v = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_T_attr_name_fsa_v_branches
    }
};

/** Symbol T attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_T_attr_name_fsa__branches[1] = {
    /* v prefix */  {
        .character = 'v',
        .sub_fsa   = &sym_T_attr_name_fsa_v
    },
};

/** Symbol T attribute names resolution FSA */
static const attr_name_fsa_t sym_T_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_T_attr_name_fsa__branches
    }
};

/** Symbol E attribute value resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_value_leaf = {
    .locator = {
        .index = 0,
    }
};

/** Symbol E attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_E_attr_name_fsa_value_branches[1] = {
    /* value (done) */  {
        .character = '\0',
        .sub_fsa   = &sym_E_attr_name_fsa_value_leaf
    },
};

/** Symbol E attribute names resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_value = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_E_attr_name_fsa_value_branches
    }
};

/** Symbol E attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_E_attr_name_fsa_valu_branches[1] = {
    /* value prefix */  {
        .character = 'e',
        .sub_fsa   = &sym_E_attr_name_fsa_value
    },
};

/** Symbol E attribute names resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_valu = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_E_attr_name_fsa_valu_branches
    }
};

/** Symbol E attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_E_attr_name_fsa_val_branches[1] = {
    /* valu prefix */  {
        .character = 'u',
        .sub_fsa   = &sym_E_attr_name_fsa_valu
    },
};

/** Symbol E attribute names resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_val = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_E_attr_name_fsa_val_branches
    }
};

/** Symbol E attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_E_attr_name_fsa_va_branches[1] = {
    /* val prefix */  {
        .character = 'l',
        .sub_fsa   = &sym_E_attr_name_fsa_val
    },
};

/** Symbol E attribute names resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_va = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_E_attr_name_fsa_va_branches
    }
};

/** Symbol E attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_E_attr_name_fsa_v_branches[1] = {
    /* va prefix */  {
        .character = 'a',
        .sub_fsa   = &sym_E_attr_name_fsa_va
    },
};

/** Symbol E attribute names resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_v = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_E_attr_name_fsa_v_branches
    }
};

/** Symbol E attribute names resolution FSA branches */
static const attr_name_fsa_branch_t sym_E_attr_name_fsa__branches[1] = {
    /* v prefix */  {
        .character = 'v',
        .sub_fsa   = &sym_E_attr_name_fsa_v
    },
};

/** Symbol E attribute names resolution FSA */
static const attr_name_fsa_t sym_E_attr_name_fsa_ = {
    .node = {
        .branch_cnt = 1,
        .branches   = &sym_E_attr_name_fsa__branches
    }
};

/** Non-terminals attr. names resolution FSA list */
static const attr_name_fsa_t *ntsyms_attr_names_fsa[3] = {
    /* F */  &sym_F_attr_name_fsa_,
    /* T */  &sym_T_attr_name_fsa_,
    /* E */  &sym_E_attr_name_fsa_,
};


/** Grammar symbols attribute definitions table */
const attribute_tab_t test_attribute_tab = {
    .tsyms_classes     = &tsyms_attr_classes,
    .ntsyms_classes    = &ntsyms_attr_classes,
    .tsyms_expl_evals  = &tsyms_attr_eval_descrs,
    .ntsyms_expl_evals = &ntsyms_attr_eval_descrs,
    .tsyms_names_fsa   = &tsyms_attr_names_fsa,
    .ntsyms_names_fsa  = &ntsyms_attr_names_fsa,
};


/*
 * Grammar rules-specific references to attr. evaluators
 */

/** Grammar rule #1 left-hand side symbol (E) attrs (1) evaluators */
static const attr_eval_descr_refs_t rule1_lhs_attr_evals = {
    &sym_E_idx0_attr_value_rule1_eval_descr,
};

/** Grammar rule #2 left-hand side symbol (E) attrs (1) evaluators */
static const attr_eval_descr_refs_t rule2_lhs_attr_evals = {
    &sym_E_idx0_attr_value_rule2_eval_descr,
};

/** Grammar rule #3 left-hand side symbol (F) attrs (1) evaluators */
static const attr_eval_descr_refs_t rule3_lhs_attr_evals = {
    &sym_F_idx0_attr_value_rule3_eval_descr,
};

/** Grammar rule #4 left-hand side symbol (F) attrs (1) evaluators */
static const attr_eval_descr_refs_t rule4_lhs_attr_evals = {
    &sym_F_idx0_attr_value_rule4_eval_descr,
};

/** Grammar rule #5 left-hand side symbol (T) attrs (1) evaluators */
static const attr_eval_descr_refs_t rule5_lhs_attr_evals = {
    &sym_T_idx0_attr_value_rule5_eval_descr,
};

/** Grammar rule #6 left-hand side symbol (T) attrs (1) evaluators */
static const attr_eval_descr_refs_t rule6_lhs_attr_evals = {
    &sym_T_idx0_attr_value_rule6_eval_descr,
};


/*
 * Grammar rules
 */

/** Grammar rule #0: _E => E */
static const grammar_rule_t rule0 = {
    .number      = 0,
    .lhs_nt      = NT__E,
    .rhs_sym_cnt = 1,
    .lhs_attr_evals =
        /* _E */  NULL,
    .rhs_attr_evals = {
        /*  E */  NULL,
    }
};

/** Grammar rule #1: E => E add F */
static const grammar_rule_t rule1 = {
    .number      = 1,
    .lhs_nt      = NT_E,
    .rhs_sym_cnt = 3,
    .lhs_attr_evals =
        /*   E */  &rule1_lhs_attr_evals,
    .rhs_attr_evals = {
        /*   E */  NULL,
        /* add */  NULL,
        /*   F */  NULL,
    }
};

/** Grammar rule #2: E => F */
static const grammar_rule_t rule2 = {
    .number      = 2,
    .lhs_nt      = NT_E,
    .rhs_sym_cnt = 1,
    .lhs_attr_evals =
        /* E */  &rule2_lhs_attr_evals,
    .rhs_attr_evals = {
        /* F */  NULL,
    }
};

/** Grammar rule #3: F => F mul T */
static const grammar_rule_t rule3 = {
    .number      = 3,
    .lhs_nt      = NT_F,
    .rhs_sym_cnt = 3,
    .lhs_attr_evals =
        /*   F */  &rule3_lhs_attr_evals,
    .rhs_attr_evals = {
        /*   F */  NULL,
        /* mul */  NULL,
        /*   T */  NULL,
    }
};

/** Grammar rule #4: F => T */
static const grammar_rule_t rule4 = {
    .number      = 4,
    .lhs_nt      = NT_F,
    .rhs_sym_cnt = 1,
    .lhs_attr_evals =
        /* F */  &rule4_lhs_attr_evals,
    .rhs_attr_evals = {
        /* T */  NULL,
    }
};

/** Grammar rule #5: T => lpar E rpar */
static const grammar_rule_t rule5 = {
    .number      = 5,
    .lhs_nt      = NT_T,
    .rhs_sym_cnt = 3,
    .lhs_attr_evals =
        /*    T */  &rule5_lhs_attr_evals,
    .rhs_attr_evals = {
        /* lpar */  NULL,
        /*    E */  NULL,
        /* rpar */  NULL,
    }
};

/** Grammar rule #6: T => number */
static const grammar_rule_t rule6 = {
    .number      = 6,
    .lhs_nt      = NT_T,
    .rhs_sym_cnt = 1,
    .lhs_attr_evals =
        /*      T */  &rule6_lhs_attr_evals,
    .rhs_attr_evals = {
        /* number */  NULL,
    }
};


/** Rule table */
const lr1_rule_tab_t test_rule_tab = {
    .rule_cnt = 7,
    .impl     = {
        /* 0 */  &rule0,
        /* 1 */  &rule1,
        /* 2 */  &rule2,
        /* 3 */  &rule3,
        /* 4 */  &rule4,
        /* 5 */  &rule5,
        /* 6 */  &rule6,
    }
};


/*
 * Attribute evaluators wrappers
 */

/** mul attribute evaluator wrapper */
static attr_eval_status_t external__mul__arity2(
    attr_handle_t      *lattr,
    attr_handle_refs_t *rattrs)
{
    assert(NULL != lattr);
    assert(NULL != rattrs);

    attr_eval_status_t status = ATTR_EVAL_ERROR;

    void *val;

    assert(NULL != (*rattrs)[0]);
    assert(ATTR_EVAL_OK == attr_get_eval_status((*rattrs)[0]));
    void *arg0 = attr_get_value((*rattrs)[0]);

    assert(NULL != (*rattrs)[1]);
    assert(ATTR_EVAL_OK == attr_get_eval_status((*rattrs)[1]));
    void *arg1 = attr_get_value((*rattrs)[1]);

    status = mul(&val, arg0, arg1);

    attr_set_eval_status(lattr, status);

    if (ATTR_EVAL_OK == status)
        attr_set_value(lattr, val);

    return status;
}


/** sum attribute evaluator wrapper */
static attr_eval_status_t external__sum__arity2(
    attr_handle_t      *lattr,
    attr_handle_refs_t *rattrs)
{
    assert(NULL != lattr);
    assert(NULL != rattrs);

    attr_eval_status_t status = ATTR_EVAL_ERROR;

    void *val;

    assert(NULL != (*rattrs)[0]);
    assert(ATTR_EVAL_OK == attr_get_eval_status((*rattrs)[0]));
    void *arg0 = attr_get_value((*rattrs)[0]);

    assert(NULL != (*rattrs)[1]);
    assert(ATTR_EVAL_OK == attr_get_eval_status((*rattrs)[1]));
    void *arg1 = attr_get_value((*rattrs)[1]);

    status = sum(&val, arg0, arg1);

    attr_set_eval_status(lattr, status);

    if (ATTR_EVAL_OK == status)
        attr_set_value(lattr, val);

    return status;
}


/** builtin::get_token attribute evaluator wrapper */
static attr_eval_status_t builtin__get_token__arity0(
    attr_handle_t      *lattr,
    attr_handle_refs_t *rattrs)
{
    assert(NULL != lattr);

    attr_eval_status_t status = ATTR_EVAL_ERROR;

    void *val;

    status = builtin__get_token(lattr, &val);

    attr_set_eval_status(lattr, status);

    if (ATTR_EVAL_OK == status)
        attr_set_value(lattr, val);

    return status;
}


/** token2num attribute evaluator wrapper */
static attr_eval_status_t external__token2num__arity1(
    attr_handle_t      *lattr,
    attr_handle_refs_t *rattrs)
{
    assert(NULL != lattr);
    assert(NULL != rattrs);

    attr_eval_status_t status = ATTR_EVAL_ERROR;

    void *val;

    assert(NULL != (*rattrs)[0]);
    assert(ATTR_EVAL_OK == attr_get_eval_status((*rattrs)[0]));
    void *arg0 = attr_get_value((*rattrs)[0]);

    status = token2num(&val, arg0);

    attr_set_eval_status(lattr, status);

    if (ATTR_EVAL_OK == status)
        attr_set_value(lattr, val);

    return status;
}


/** Reference attribute evaluator */
static attr_eval_status_t builtin__reference__arity1(
    attr_handle_t      *lattr,
    attr_handle_refs_t *rattrs)
{
    assert(NULL != lattr);
    assert(NULL != rattrs);
    assert(NULL != (*rattrs)[0]);

    assert(ATTR_EVAL_OK == attr_get_eval_status((*rattrs)[0]));

    attr_set_value(lattr, attr_get_value((*rattrs)[0]));

    attr_set_reference_flag(lattr);

    attr_set_eval_status(lattr, ATTR_EVAL_OK);

    return ATTR_EVAL_OK;
}


/*
 * Attribute destructors wrappers
 */

/** destroy_value attribute destructor wrapper */
static void external__destroy_value__arity1(attr_handle_t *attr) {
    assert(NULL != attr);

    if (ATTR_EVAL_OK == attr_get_eval_status(attr))
        destroy_value(attr_get_value(attr));
}


