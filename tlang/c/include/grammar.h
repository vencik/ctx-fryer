#ifndef CTXFryer__grammar_h
#define CTXFryer__grammar_h

/**
 *  \brief  Grammar
 *
 *  Definition of grammar info needed for parsing, syntax error reporting etc;
 *  in short, whatever information necessary to bind parsing with the source
 *  grammar definition.
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/07/16
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

#include <stddef.h>


typedef struct grammar_rule grammar_rule_t;  /**< Grammar rule  info */
typedef struct rule_attr    rule_attr_t;     /**< Grammar attr. info */


/** Grammar rule info */
struct grammar_rule {
    size_t number;       /**< Rule number                            */
    int    lhs_nt;       /**< Rule left  hand side non-terminal code */
    size_t rhs_sym_cnt;  /**< Rule right hand side symbol count      */

    /** Rule-specific attribution info */
    const attr_eval_descr_refs_t *lhs_attr_evals;    /**< LHS sym. attr. evals */
    const attr_eval_descr_refs_t *rhs_attr_evals[];  /**< RHS sym. attr. evals */
};  /* end of struct grammar_rule */


/*
 * Grammar rule info interface
 */

/**
 *  \brief  Grammar rule number
 *
 *  \param  rule  Grammar rule info
 *
 *  \return Rule number
 */
#define grammar_rule_no(rule) ((rule)->number)


/**
 *  \brief  Grammar rule left hand side non-terminal
 *
 *  \param  rule  Grammar rule info
 *
 *  \return Rule LHS non-terminal
 */
#define grammar_rule_lhs_non_terminal(rule) ((rule)->lhs_nt)


/**
 *  \brief  Grammar rule right hand side sentential form length
 *
 *  \param  rule  Grammar rule info
 *
 *  \return Count of the rule RHS symbols
 */
#define grammar_rule_rhs_symbol_count(rule) ((rule)->rhs_sym_cnt)


/**
 *  \brief  Grammar symbol n-th attribute evaluator
 *
 *  The macro provides n-th attribute evaluator selected
 *  from an array of symbol attributes evaluators.
 *  If the array isn't defined, \c NULL is provided.
 *  Otherwise, n-th item of the array is provided
 *  (the item may also be \c NULL).
 *
 *  The macro is to be considered private to this module.
 *
 *  \param  evals  Address of an array of attributes evaluators
 *  \param  n      Attribute index
 *
 *  \return Attribute evaluator or \c NULL in case it's not defined
 */
#define _grammar_rule_attr_eval(evals, n) \
    (NULL != (evals) ? (*(evals))[(n)] : NULL)


/**
 *  \brief  Grammar rule left hand side non-terminal n-th attribute evaluator
 *
 *  \param  rule  Grammar rule info
 *  \param  n     Attribute index (0-based)
 *
 *  \return Rule-specific attribute evaluator descriptor or \c NULL if not set
 */
#define grammar_rule_lhs_attr_eval(rule, n) \
    (assert(NULL != (rule)), \
     _grammar_rule_attr_eval((rule)->lhs_attr_evals, (n)))


/**
 *  \brief  Grammar rule right hand side k-th symbol n-th attribute evaluator
 *
 *  \param  rule  Grammar rule info
 *  \param  k     Symbol index (0-based)
 *  \param  n     Attribute index (0-based)
 *
 *  \return Rule-specific attribute evaluator descriptor or \c NULL if not set
 */
#define grammar_rule_rhs_attr_eval(rule, k, n) \
    (assert(NULL != (rule)), \
     assert((k) < ((rule)->rhs_sym_cnt)), \
     _grammar_rule_attr_eval((rule)->rhs_attr_evals[(k)], (n))

#endif /* end of #ifndef CTXFryer__grammar_h */
