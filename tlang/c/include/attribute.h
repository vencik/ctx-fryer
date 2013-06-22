#ifndef CTXFryer__attribute_h
#define CTXFryer__attribute_h

/**
 *  \brief  Grammar attribution
 *
 *  In the theory of formal languages and automata, attributed grammars
 *  are used to model the process of transforming the source (syntax)
 *  to meaning (semantics) by evaluation of the grammar symbols attributes.
 *
 *  Each symbol of the grammar has a set of attributes.
 *  There are 2 types of attributes: inherited and aggregated.
 *  Attributes are evaluated during the parse tree traversal;
 *  imagine now, that we traverse the parse tree to depth.
 *  Inherited attribute is an attribute which value is derived from attributes
 *  evaluated before the symbol parse tree node was visited during the traversal
 *  i.e. "on the way down" during the tree traversal.
 *  Likewise, aggregated attribute is an attribute which value is derived from
 *  attributes evaluated before the symbol parse tree node was re-visited
 *  while backtracking, i.e. "on the way up".
 *
 *  The above yields a few simple observations when considering LR
 *  (i.e. bottom-to-top) syntax analysis:
 *  1/ Since the parse tree is built from leaves to root, evaluation of aggregated
 *     attributes can be done during parsing, while evaluation of inherited
 *     attributes can't
 *  2/ We therefore probably need to do a posteriori parse tree traversal
 *  3/ Considering 2/, we might even consider multiple parse tree traversals.
 *     This might ressemble the multiple different stages of the code
 *     processing as represented e.g. by \c BEGIN / \c INIT etc. blocks used
 *     in Perl
 *
 *  Attribute type is a static property of an attribute.
 *  That means that within the same grammar, a symbol attribute must be
 *  aggregated/inherited in all rules that evaluate it.
 *  Although it could be technically possible to allow different evaluation
 *  dependencies direction in different rules, it would
 *  1/ taint our attributed grammars theory implementation
 *  2/ bite a bit out of the evaluation efficiency and code clarity
 *  There's a good reason to the attribute (meeta) type dichotomy:
 *  All aggregated attributes dependencies may be resolved as soon as
 *  the associated parse tree node is created (i.e. at time of a shift action
 *  or reduction by a rule with LHS being the symbol in question).
 *  Inherited attributes dependencies, on the other hand, may be resolved
 *  in general only after all syblings and the parent in the parse tree were
 *  created.
 *  It is therefore natural to split the attributes to 2 groups, each with
 *  different time of resolution of dependencies.
 *  The evaluation is then a generic process.
 *
 *  To maintain high efficiency, attribute evaluation is in fact done on-demand
 *  (transitively).
 *  For real world grammars that contain quite a lot of syntax suggar
 *  which produces parse tree nodes without (interresting) attributes,
 *  the "lazy" on-demand approach can save considerable amount of operations.
 *  That is especially true for heavily segmented input, which requires
 *  defragmentation of lexical items (that is necessary for evaluation of
 *  their token attributes).
 *  In the case of syntax suggar, chances are very high that nobody ever
 *  requests these attributes (although, in general, they might).
 *
 *  Each attribute back-references the parse tree node that owns it.
 *  The bidirectional bound is necessary to provide built-in attr. evaluators
 *  information from parse tree itself.
 *  The built-in attribute evaluators use the very same mechanism of execution
 *  as the user-defined ones.
 *  The only difference being that they have access to the parse tree.
 *  Although the bidirectional bound is somewhat of a blot on the data model
 *  design, it allows for united and generic approach to built-in attribute
 *  evaluation, sharing the on-demand attr. evaluation mechanism with
 *  user-defined attributes and easy addition of new built-in attribute
 *  evaluators if required in the future without compromising compatibility
 *  with existing definitions.
 *  This appears to be reasonable enough justification.
 *
 *  The file is part of CTX Fryer C target language libraries.
 *
 *  \date  2012/09/24
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

#include "objpack.h"
#include "ptree_types.h"

#include <stddef.h>
#include <stdint.h>


typedef struct attr_handle      attr_handle_t;       /**< Attribute handle       */
typedef struct attr_class_descr attr_class_descr_t;  /**< Attr. class descriptor */
typedef struct attr_classes     attr_classes_t;      /**< Attr. classes          */
typedef struct attr_eval_descr  attr_eval_descr_t;   /**< Evaluator descr.       */


/** Attribute type (aggregated/inherited) */
typedef enum {
    ATTR_TYPE_AGGREGATED,  /**< Aggregated attribute */
    ATTR_TYPE_INHERITED,   /**< Inherited  attribute */
} attr_type_t;  /* end of typedef enum */


/** Attribute evaluation status (mutually exclusive) */
typedef enum {
    ATTR_EVAL_OK    = 0,  /**< Evaluation was successfull  */
    ATTR_EVAL_UNDEF = 1,  /**< Attribute value not defined */
    ATTR_EVAL_ERROR = 2,  /**< Evaluation failed           */
} attr_eval_status_t;  /* end of typedef enum */


/** Attribute dependencies status flags */
typedef enum {
    ATTR_DEPS_INVALID    = 0x00,  /**< Dependencies not allocated     */
    ATTR_DEPS_INIT       = 0x01,  /**< Dependencies allocated         */
    ATTR_DEPS_RESOLVED   = 0x02,  /**< Dependencies were resolved     */
    ATTR_DEPS_EVAL_SCHED = 0x04,  /**< Deps. evaluation was scheduled */
    ATTR_DEPS_EVALUATED  = 0x08,  /**< Dependencies were evaluated    */
} attr_deps_status_t;  /* end of typedef enum */


/** Miscelaneous attribute flags */
typedef enum {
    ATTR_IS_REF = 0x0001,  /**< Attribute is a reference */
} attr_flags_t;  /* end of typedef enum */


typedef attr_handle_t * attr_handle_refs_t[];  /**< Attribute handle refs */


/** Attribute handle */
struct attr_handle {
    uint32_t                  st_flags;     /**< Attribute (packed) status flags */
    ptree_node_t             *ptnode;       /**< Parse-tree node back-reference  */
    objpack_handle_t         *pack;         /**< Attribute pack it belongs to    */
    const attr_class_descr_t *class_descr;  /**< Attribute class descriptor      */
    const attr_eval_descr_t  *eval_descr;   /**< Attribute evaluation descriptor */
    void                     *val;          /**< Attribute value                 */
    objpack_handle_t         *dep_pack;     /**< Attribute dependencies pack     */
    attr_handle_refs_t       *deps;         /**< Attribute dependencies          */
};  /* end of struct attr_handle */


typedef attr_handle_t attr_handles_t[];  /**< Attribute handles     */


/**
 *  \brief  Attribute evaluator wrapper
 *
 *  Attribute evaluation function wrapper.
 *  The generator produces headers and wrappers around real
 *  (either user-defined or built-in) attribute evaluators.
 *
 *  The real evaluator takes n + 1 arguments (n being the evaluator
 *  arity).
 *  The 1st (output) argument is a pointer to the evaluated attribute
 *  value (formally of type \c void \c**).
 *  Of course, that means that the user MUST NOT provide pointer
 *  to a local variable.
 *  See \ref attr_destructor_t for info about destruction of the attribute
 *  values.
 *  The next n arguments are pointers to arguments (\v void \c *).
 *  The real evaluator is expected to return an \ref attr_eval_status_t
 *  value accordingly to the evaluation result.
 *
 *  The above mechanism allows for minimal binding between the generated
 *  and hand-written code.
 *  That's important to allow future changes in the code generator
 *  (if required) while keeping backward compatibility of the new generated
 *  code and the former hand-written code.
 *  That is in hand necessary to enable users to extend and/or change their
 *  language grammar, compile the result with a new version of the generator
 *  and still be able to use their old evaluators without needing to change
 *  them.
 *
 *  \param  lattr   Evaluated attribute
 *  \param  rattrs  Attributes used for evaluation
 *
 *  \retval ATTR_EVAL_OK    if attribute was successfully evaluated
 *  \retval ATTR_EVAL_UNDEF if attribute couldn't be evaluated
 *  \retval ATTR_EVAL_ERROR in case of error
 */
typedef attr_eval_status_t attr_evaluator_t(attr_handle_t *lattr, attr_handle_refs_t *rattrs);


/**
 *  \brief  Attribute value destructor wrapper
 *
 *  If an attribute value is explicitly defined in the specification,
 *  it shall be executed (via a generated wrapper of this type)
 *  at due time.
 *
 *  The actual value destructor is expected to be a function
 *  with 1 argument of type \c void \c* (a pointer to the attribute
 *  value).
 *  Its return value is ignored (destructors shouldn't ever fail).
 *
 *  Note that attribute value destructor shall ONLY be executed
 *  for an attribute which evaluation status equals to \c ATTR_EVAL_OK.
 *  If the evaluation of the attribute fails (i.e. \c ATTR_EVAL_ERROR
 *  is returned), the evaluator is expected to cleanup any dynamic memory
 *  allocated for the value.
 *
 *  \param  attr  Attribute
 */
typedef void attr_destructor_t(attr_handle_t *attr);


/** Attribute class desciptor */
struct attr_class_descr {
    attr_type_t        type;      /**< Attribute type             */
    const char        *id;        /**< Attribute identifier       */
    attr_destructor_t *destroy;   /**< Attribute value destructor */
};  /* end of struct attr_class_descr */


typedef attr_class_descr_t attr_class_descrs_t[];  /**< Attr. class descriptors */


/** Attribute classes (for a symbol) */
struct attr_classes {
    size_t                     class_cnt;  /**< Attr. class count       */
    const attr_class_descrs_t *classes;    /**< Attr. class descriptors */
};  /* end of struct attr_classes */


/** Attribute dependency descriptor */
struct attr_dep_descr {
    size_t sym_idx;   /** Symbol index           */
    size_t attr_idx;  /** Symbol attribute index */
};  /* end of struct attr_dep_descr */

typedef struct attr_dep_descr attr_dep_descr_t;  /**< Attr. dependency descriptor */

/** Attribute evaluation descriptor */
struct attr_eval_descr {
    attr_evaluator_t *eval;          /**< Attribute evaluator         */
    size_t            dep_cnt;       /**< Dependency attributes count */
    attr_dep_descr_t  dep_descrs[];  /**< Dependency descriptors      */
};  /* end of struct attr_eval_descr */


typedef const attr_eval_descr_t   attr_eval_descrs_t[];      /**< Evaluator descrs.     */
typedef const attr_eval_descr_t * attr_eval_descr_refs_t[];  /**< Evaluator descr. refs */


typedef union  attr_name_fsa         attr_name_fsa_t;          /**< Attribute name FSA    */
typedef struct attr_name_fsa_node    attr_name_fsa_node_t;     /**< Attr. name FSA node   */
typedef struct attr_name_fsa_branch  attr_name_fsa_branch_t;   /**< Attr. name FSA branch */
typedef struct attr_name_fsa_locator attr_name_fsa_locator_t;  /**< Attr. locator info    */


/** Attribute name FSA branch */
struct attr_name_fsa_branch {
    char                   character;  /**< Attribute name character */
    const attr_name_fsa_t *sub_fsa;    /**< Attribute name (sub-)FSA */
};  /* end of struct attr_name_fsa_branch */


typedef attr_name_fsa_branch_t attr_name_fsa_branches_t[];  /**< Attr. name FSA branches */


/** Attribute name FSA node */
struct attr_name_fsa_node {
    size_t                          branch_cnt;  /**< Branch count                    */
    const attr_name_fsa_branches_t *branches;    /**< Branches (ordered by character) */
};  /* end of atruct attr_name_fsa_node */


/** Attribute name FSA locator info */
struct attr_name_fsa_locator {
    size_t index;  /**< Index in the symbol attr. list */
};  /* end of struct attr_name_fsa_locator */


/**
 *  \brief  Attribute name finite state automaton (node)
 *
 *  The union represents both the attribute name FSA inner and terminal node.
 *  The FSA is used to map a grammar symbol attribute names to their indices
 *  in the symbol attribute list and therefore to efficiently resolve a symbol
 *  attribute by its name.
 */
union attr_name_fsa {
    attr_name_fsa_node_t    node;     /**< Attribute name FSA node */
    attr_name_fsa_locator_t locator;  /**< Attribute locator       */
};  /* end of union attr_name_fsa */


/**
 *  \brief  Attribute evaluation status getter
 *
 *  Provides attribute eval. status as returned by its evaluator
 *  or \c ATTR_EVAL_UNDEF in case evaluation wasn't done, yet.
 *
 *  \param  attr  Attribute
 *
 *  \return Attribute status
 */
#define attr_get_eval_status(attr) \
    ((attr_eval_status_t)((attr)->st_flags & 0x000000ff))


/**
 *  \brief  Attribute evaluation status setter
 *
 *  \param  attr    Attribute
 *  \param  status  Attribute evaluation status
 */
#define attr_set_eval_status(attr, status) \
    (((attr)->st_flags &= 0xffffff00), ((attr)->st_flags |= (uint32_t)(status)))


/**
 *  \brief  Attribute dependencies status flags getter
 *
 *  INTERNAL MACRO.
 *  Should not be called outside the attribute module.
 *
 *  \param  attr  Attribute
 *
 *  \return Attribute dependencies status flags
 */
#define _attr_get_depend_flags(attr) \
    (((attr)->st_flags & 0x0000ff00) >> 8)


/**
 *  \brief  Set attribute dependencies status flags
 *
 *  INTERNAL MACRO.
 *  Should not be called outside the attribute module.
 *
 *  \param  attr    Attribute
 *  \param  status  Attribute dependencies status flags
 */
#define _attr_set_depend_flags(attr, status) \
    ((attr)->st_flags |= ((uint32_t)(status) << 8))


/**
 *  \brief  Clear attribute dependencies status flags
 *
 *  INTERNAL MACRO.
 *  Should not be called outside the attribute module.
 *
 *  \param  attr    Attribute
 *  \param  status  Attribute dependencies status flags mask
 */
#define _attr_clear_depend_flags(attr, status) \
    ((attr)->st_flags &= ~((uint32_t)(status) << 8))


/** Dependencies initialised quory */
#define attr_is_depend_init(attr) \
    (_attr_get_depend_flags(attr) & (uint32_t)ATTR_DEPS_INIT)

/** Dependencies resolved quory */
#define attr_is_depend_resolved(attr) \
    (_attr_get_depend_flags(attr) & (uint32_t)ATTR_DEPS_RESOLVED)

/** Dependencies scheduled for evaluation */
#define attr_is_depend_eval_scheduled(attr) \
    (_attr_get_depend_flags(attr) & (uint32_t)ATTR_DEPS_EVAL_SCHED)

/** Dependencies evaluated quory */
#define attr_is_depend_evaluated(attr) \
    (_attr_get_depend_flags(attr) & (uint32_t)ATTR_DEPS_EVALUATED)


/** Dependencies initialised */
#define attr_set_depend_init(attr) \
    _attr_set_depend_flags((attr), ATTR_DEPS_INIT)

/** Dependencies resolved */
#define attr_set_depend_resolved(attr) \
    _attr_set_depend_flags((attr), ATTR_DEPS_RESOLVED)

/** Dependencies scheduled for evaluation */
#define attr_set_depend_eval_scheduled(attr) \
    _attr_set_depend_flags((attr), ATTR_DEPS_EVAL_SCHED)

/** Dependencies evaluated */
#define attr_set_depend_evaluated(attr) \
    _attr_set_depend_flags((attr), ATTR_DEPS_EVALUATED)


/** Clear all dependency flags */
#define attr_clear_depend_flags(attr) \
    _attr_clear_depend_flags((attr), 0xff)

/** Dependencies not initialised */
#define attr_clear_depend_init(attr) \
    _attr_clear_depend_flags((attr), ATTR_DEPS_INIT)

/** Dependencies not resolved */
#define attr_clear_depend_resolved(attr) \
    _attr_clear_depend_flags((attr), ATTR_DEPS_RESOLVED)

/** Dependencies not scheduled for evaluation */
#define attr_clear_depend_eval_scheduled(attr) \
    _attr_clear_depend_flags((attr), ATTR_DEPS_EVAL_SCHED)

/** Dependencies not evaluated */
#define attr_clear_depend_evaluated(attr) \
    _attr_clear_depend_flags((attr), ATTR_DEPS_EVALUATED)


/**
 *  \brief  Miscelaneous attribute status flags getter
 *
 *  INTERNAL MACRO.
 *  Should not be called outside the attribute module.
 *
 *  \param  attr  Attribute
 *
 *  \return Misc. attribute status flags
 */
#define _attr_get_misc_flags(attr) \
    ((attr)->st_flags >> 16)


/**
 *  \brief  Set miscelaneous attribute status flags
 *
 *  INTERNAL MACRO.
 *  Should not be called outside the attribute module.
 *
 *  \param  attr    Attribute
 *  \param  status  Misc. attribute status flags
 */
#define _attr_set_misc_flags(attr, status) \
    ((attr)->st_flags |= ((uint32_t)(status) << 16))


/**
 *  \brief  Clear miscelaneous attribute status flags
 *
 *  INTERNAL MACRO.
 *  Should not be called outside the attribute module.
 *
 *  \param  attr    Attribute
 *  \param  status  Misc. attribute status flags mask
 */
#define _attr_clear_misc_flags(attr, status) \
    ((attr)->st_flags &= ~((uint32_t)(status) << 16))


/**
 *  \brief  Attribute is a reference
 *
 *  Attribute just references another one.
 *  Destructor will not be called.
 *
 *  \param  attr  Attribute
 *
 *  \retval Non-zero if the attribute is a reference
 *  \retval 0        otherwise
 */
#define attr_is_reference(attr) \
    (_attr_get_misc_flags(attr) & (uint32_t)ATTR_IS_REF)


/**
 *  \brief  Set attribute reference flag
 *
 *  Mark attribute as a reference.
 *
 *  \param  attr  Attribute
 */
#define attr_set_reference_flag(attr) \
    _attr_set_misc_flags((attr), ATTR_IS_REF)


/**
 *  \brief  Clear attribute reference flag
 *
 *  Unmark attribute as a reference.
 *
 *  \param  attr  Attribute
 */
#define attr_clear_reference_flag(attr) \
    _attr_clear_misc_flags((attr), ATTR_IS_REF)


/**
 *  \brief  Get attribute parse tree node (i.e. owner)
 *
 *  \param  attr  Attribute
 *
 *  \return Parse tree node
 */
#define attr_ptree_node(attr) ((attr)->ptnode)


/**
 *  \brief  Attribute value getter
 *
 *  Provides attribute value as set by its evaluator
 *  or \c NULL if it isn't evaluated, yet.
 *  Note that this this macro isn't enough to decide
 *  whether an attribute was evaluated or not; \c NULL
 *  may even be valid evaluation or there might've been
 *  evaluation error.
 *  Use \ref attr_eval_status to check the attribute status,
 *  first.
 *
 *  \param  attr  Attribute
 *
 *  \return Attribute value (may be \c NULL)
 */
#define attr_get_value(attr) ((attr)->val)


/**
 *  \brief  Attribute value setter
 *
 *  Sets attribute value.
 *
 *  \param  attr   Attribute
 *  \param  value  Attribute value
 *
 *  \retval ATTR_EVAL_OK
 */
#define attr_set_value(attr, value) ((attr)->val = (value))


/**
 *  \brief  Attribute dependencies count getter
 *
 *  \param  attr  Attribute
 *
 *  \return Number of evaluation dependencies
 */
#define attr_get_depend_count(attr) ((attr)->eval_descr->dep_cnt)


/**
 *  \brief  Attribute dependency symbol index getter
 *
 *  \param  attr  Attribute
 *  \param  n     Dependency index
 *
 *  \return Symbol index
 */
#define attr_get_depend_symbol_index(attr, n) ((attr)->eval_descr->dep_descrs[(n)].sym_idx)


/**
 *  \brief  Attribute dependency attribute index getter
 *
 *  \param  attr  Attribute
 *  \param  n     Dependency index
 *
 *  \return Attribute index
 */
#define attr_get_depend_attr_index(attr, n) ((attr)->eval_descr->dep_descrs[(n)].attr_idx)


/**
 *  \brief  Attribute dependency getter
 *
 *  \param  attr   Attribute
 *  \param  index  Dependency index
 *
 *  \return Evaluation dependency
 */
#define attr_get_depend(attr, index) \
    (assert((index) < attr_get_depend_count(attr)), (attr)->deps[index])


/**
 *  \brief  Attribute dependency setter
 *
 *  \param  attr   Attribute
 *  \param  index  Dependency index
 *  \param  dep    Dependency
 */
#define attr_set_depend(attr, index, dep) \
    do { \
        assert((index) < attr_get_depend_count(attr)); \
        (*(attr)->deps)[index] = (dep); \
    } while (0)


/**
 *  \brief  Attribute destructor getter
 *
 *  \param  attr  Attribute
 */
#define attr_destructor(attr) ((attr)->class_descr->destroy)


/**
 *  \brief  Attribute(s) constructor
 *
 *  Creates attribute handles.
 *  Note that attributes dependencies are NOT set (since the dependencies
 *  may not exist yet in the time of attributes creation).
 *
 *  Attribute descriptors MUST be provided pre-sorted by (meta) type,
 *  aggregated first.
 *  Since attribute (meta) type is static info, this requirement is not
 *  a real constraint.
 *
 *  Two sets of evaluators may be provided (\c NULL is acceptable if any
 *  of the sets doesn't exist at all).
 *  The 1st set represents attribute evaluators defined generically for
 *  a symbol attribute, regardless on the context (i.e. grammar rule).
 *  The 2nd set provides evaluators defined specifically per-rule.
 *  Note that ideally, the sets should be disjoint (i.e. exactly one
 *  evaluator should be defined per each attribute).
 *  If both generic and specific evaluators are defined, though, the specific
 *  is preferred and accepted.
 *  If no evaluator is defined then the attribute can't be evaluated of course,
 *  which strongly suggests faulty/inadequate attribute set definition.
 *
 *  Each attribute gets back-reference to parse tree node it belongs to.
 *  This is (unfortunately) necessary to access parsing info (like lexical
 *  items) to provide data to built-in attribute evaluators.
 *  That is, AND SHALL STAY, the sole reason for the bidirectional bound.
 *
 *  \param[out]  aggreg_attr_cnt  Aggregated attributes count
 *  \param[in]   ptnode           Parse tree node back-reference
 *  \param[in]   cnt              Attribute count
 *  \param[in]   classes          Attribute classes descriptors
 *  \param[in]   g_evals          Generic attributes evaluation descriptor refs
 *  \param[in]   s_evals          Specific attributes evaluation descriptor refs
 *
 *  \return Attribute handles or \c NULL if they couldn't be created
 */
attr_handles_t *attr_create(
    size_t                       *aggreg_attr_cnt,
    ptree_node_t                 *ptnode,
    size_t                        cnt,
    const attr_class_descrs_t    *classes,
    const attr_eval_descr_refs_t *g_evals,
    const attr_eval_descr_refs_t *s_evals);


/**
 *  \brief  Attribute(s) destructor
 *
 *  If the attribute was sucessfully evaluated,
 *  the value is also destroyed.
 *
 *  \param  attrs  Attributes
 *  \param  cnt    Attribute count
 */
void attr_destroy(attr_handles_t *attrs, size_t cnt);


/**
 *  \brief  Set (override) attribute(s) evaluators
 *
 *  The function is used to set attributes evaluators
 *  for already created attributes.
 *  Non-\c NULL evaluators provided shall override
 *  any existing evaluators.
 *  The function is unavoidably required for setting of
 *  inherited attributes evaluators since at time of creation
 *  of such attributes, the derivation rule (and therefore
 *  the evaluators) is not known, yet.
 *
 *  \param  attrs  Attributes
 *  \param  cnt    Attribute count
 *  \param  evals  Attribute evaluators
 *
 *  retval ATTR_EVAL_ERROR if attribute dependencies couldn't be allocated
 *  retval ATTR_EVAL_OK    if evaluators were set successfully
 */
attr_eval_status_t attr_evaluators(
    attr_handles_t               *attrs,
    size_t                        cnt,
    const attr_eval_descr_refs_t *evals);


/**
 *  \brief  Evaluate an attribute
 *
 *  The function performs on-demand deep attribute evaluation.
 *  If the attribute is already evaluated, it does nothing.
 *  Otherwise it evaluates all the attribute dependencies (unless already
 *  evaluated) and then evaluates the attribute itself.
 *  Evaluation is interrupted as soon as the first evaluator
 *  isn't successful (i.e. all dependencies must be evaluated
 *  to perform an attribute evaluation).
 *
 *  Note the \c deps_status entry of the \ref attr_handle_t attr. handle.
 *  When the attribute is being evaluated, the entry is set to
 *  \ref ATTR_DEPS_EVALUATED as soon as all its dependencies are either
 *  already evaluated or scheduled for evaluation.
 *  Checking these flags while popping evaluated attributes off the stack
 *  eliminates the need of re-checking evaluation status of all the dependencies
 *  before attempting another stacked attribute evaluation.
 *
 *  The dependency depth may be limited to avoid potential deep recursion
 *  that might be caused by cyclic dependencies (should've been detected
 *  on compile time, though).
 *  Note that in theory, any dependency path mustn't contain more
 *  than the total amount of attribute instances created
 *  (unless it contains a loop).
 *
 *  \param  attr       Attribute
 *  \param  depth_max  Maximal dependency depth (optional, 0 means unlimited)
 *
 *  \retval ATTR_EVAL_OK    if evaluation was successfull
 *  \retval ATTR_EVAL_UNDEF if attribute value could not be defined at this stage
 *  \retval ATTR_EVAL_ERROR in case of evaluation failure
 */
attr_eval_status_t attr_eval(attr_handle_t *attr, size_t depth_max);


/**
 *  \brief  Get attribute by name
 *
 *  The function resolves attribute handle based on attribute name.
 *
 *  \param  attrs     Attribute list
 *  \param  name_fsa  Attribute names FSA
 *  \param  name      Attribute name
 *
 *  \return Attribute handle or \c NULL if no such attribute exists
 */
attr_handle_t *attr_get(
    attr_handles_t        *attrs,
    const attr_name_fsa_t *name_fsa,
    const char            *name);

#endif /* end of #ifndef CTXFryer__attribute_h */
