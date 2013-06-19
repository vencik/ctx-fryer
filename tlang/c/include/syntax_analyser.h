#ifndef CTXFryer__syntax_analyser_h
#define CTXFryer__syntax_analyser_h

/**
 *  \brief  Syntax analyser
 *
 *  The syntax analyser is generally an LR(1) parser.
 *  Formally, it is a Push-Down Automaton (PDA) with input consisting
 *  of lexical items, state is defined by the input, stack contents
 *  and current transition Finite-State Automaton (FSA) state number.
 *  The transition function is encoded in form of action and goto
 *  table derived from the accepted Context-Free Language (CFL)
 *  grammar.
 *
 *  The parser uses general concept of lexical items which
 *  may be more complicated constructs as well as single characters.
 *  Lexical items are typically words from several regular languages
 *  and the character input stream is segmented to lexical items
 *  by dedicated lexical analyser built around a FSA.
 *  This kind of pre-processing of the raw character/byte input
 *  has the advantage that the target language grammar complexity
 *  is significantly reduced, which in turn leads to creation
 *  of less complex parser (both in state space and parsing time terms).
 *
 *  Well-designed lexical item space should minimise amount of
 *  item prefix collisions (i.e. different lexical items should best not
 *  share the same prefix) because the lexical analyser must provide
 *  all possibilities and jumps back from longer prefixes to shorter
 *  mean parsing performance penalisations.
 *
 *  The analyser supports incremental parsing (is able to keep its
 *  state during input exhaustion).
 *  It also supports work either in push or pull mode
 *  (parsing on source availability or result demand, respectively).
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2012/07/16
 */

#include "lexical_analyser.h"
#include "grammar.h"
#include "ptree.h"
#include "attribute.h"

#include <stddef.h>


#define SA_NULL_TARGET SIZE_MAX  /**< LR parser goto table invalid target */


/** Syntax analyser status codes */
typedef enum {
    SA_OK = 0,           /**< Success                         */
    SA_INPUT_EXHAUSTED,  /**< Input exhausted                 */
    SA_SYNTAX_ERROR,     /**< Word doesn't belong to language */
    SA_ERROR,            /**< General error                   */
} sa_status_t;  /* end of typedef enum */


/** LR parser action types */
typedef enum {
    SA_ACTION_SHIFT,   /**< Shift action  */
    SA_ACTION_REDUCE,  /**< Reduce action */
    SA_ACTION_ACCEPT,  /**< Accept action */
    SA_ACTION_REJECT,  /**< Reject action */
} sa_action_type_t;  /* end of typedef enum */


typedef struct sa_action sa_action_t;  /**< LR parser action          */
typedef struct sa_state  sa_state_t;   /**< Syntax analyser state     */
typedef struct sa_stack  sa_stack_t;   /**< Syntax analyser PDA stack */
typedef struct syxa      syxa_t;       /**< Syntax analyser           */

typedef struct sa_action_shift  sa_action_shift_t;   /**< Shift  action */
typedef struct sa_action_reduce sa_action_reduce_t;  /**< Reduce action */
typedef struct sa_action_accept sa_action_accept_t;  /**< Accept action */

typedef struct lr1_action_tab lr1_action_tab_t;  /**< LR(1) parser action    table */
typedef struct lr1_goto_tab   lr1_goto_tab_t;    /**< LR(1) parser goto      table */
typedef struct lr1_rule_tab   lr1_rule_tab_t;    /**< LR(1) parser rule info table */

typedef struct attribute_tab attribute_tab_t;  /**< Attribute definitions */


/** Shift action */
struct sa_action_shift {
    size_t next_state;  /**< Next state number (after the item shift) */
};  /* end of struct sa_action_shift */


/** Reduce action */
struct sa_action_reduce {
    size_t rule_no;  /**< Reduce rule number */
};  /* end of struct sa_action_reduce */


/** Accept action */
struct sa_action_accept {
};  /* end of struct sa_action_accept */


/** LR parser action */
struct sa_action {
    sa_action_type_t type;  /**< Action type */

    /** Action implementation */
    union {
        sa_action_shift_t  shift;   /**< Shift  action */
        sa_action_reduce_t reduce;  /**< Reduce action */
        sa_action_accept_t accept;  /**< Accept action */
    } t;  /* end of anonymous union */
};  /* end of struct sa_action */


/*
 * Actions interface
 */

/**
 *  \brief  Shift action initialiser
 *
 *  \param  next_state  Next state
 */
#define SA_ACTION_SHIFT_INIT(next_state) \
    { \
        .type = SA_ACTION_SHIFT, \
        .t.shift.next_state = (next_state), \
    }


/**
 *  \brief  Reduce action initialiser
 *
 *  \param  rule_no  Reduce rule number
 */
#define SA_ACTION_REDUCE_INIT(rule_no) \
    { \
        .type = SA_ACTION_REDUCE, \
        .t.reduce.rule_no = (rule_no), \
    }


/**
 *  \brief  Accept action initialiser
 */
#define SA_ACTION_ACCEPT_INIT \
    { \
        .type = SA_ACTION_ACCEPT, \
    }


/**
 *  \brief  LR action type getter
 *
 *  Note that \c NULL action stands for reject action.
 *
 *  \param  action  LR action
 *
 *  \return LR action type
 */
#define sa_action_type(action) \
    (NULL != (action) ? (action)->type : SA_ACTION_REJECT)


/**
 *  \brief  Shift action next state getter
 *
 *  \param  action  Shift action
 *
 *  \return Next state after shift
 */
#define sa_action_shift_next_state(action) ((action)->t.shift.next_state)


/**
 *  \brief  Reduce action reduce rule number getter
 *
 *  \param  action  Reduce action
 *
 *  \return Reduce rule number
 */
#define sa_action_reduce_rule_no(action) ((action)->t.reduce.rule_no)


/** LR(1) parser action table */
struct lr1_action_tab {
    size_t              state_cnt;     /**< LR(1) parser state count            */
    size_t              lex_item_cnt;  /**< Lexical items count (including EoF) */
    const sa_action_t * (* impl)[];    /**< Table implementation                */
};  /* end of struct lr1_action_tab */


/** LR(1) parser goto table */
struct lr1_goto_tab {
    size_t       state_cnt;     /**< LR(1) parser state count    */
    size_t       non_term_cnt;  /**< Grammar non-terminals count */
    const size_t (* impl)[];    /**< Table implementation        */
};  /* end of struct lr1_goto_tab */


/** LR(1) parser rule info table */
struct lr1_rule_tab {
    size_t                 rule_cnt;  /**< Grammar rule count   */
    const grammar_rule_t * impl[];    /**< Table implementation */
};  /* end of struct lr1_rule_tab */


/*
 * LR(1) parser tables interface
 */

/**
 *  \brief  LR(1) parser action table accessor
 *
 *  \param  action_tab  Action table
 *  \param  state       State
 *  \param  item_code   Lexical item code
 *
 *  \return Action to perform if in \c state
 *          and lex. item with code \c item_code
 *          is actually on the input head
 */
#define sa_action_table_at(action_tab, state, item_code) \
    (assert((state)     < (action_tab)->state_cnt), \
     assert((item_code) < (action_tab)->lex_item_cnt), \
     (*(*((action_tab)->impl) + ((action_tab)->lex_item_cnt * (state) + (item_code)))))


/**
 *  \brief  LR(1) parser goto table accessor
 *
 *  \param  goto_tab  Goto table
 *  \param  state     State
 *  \param  non_term  Grammar non-terminals count
 *
 *  \return Next state after reduce action performed in \c state
 *          following non-terminal \c non_term rule.
 */
#define sa_goto_table_at(goto_tab, state, non_term) \
    (assert((state)    < (goto_tab)->state_cnt), \
     assert((non_term) < (goto_tab)->non_term_cnt), \
     (*(*((goto_tab)->impl) + ((goto_tab)->non_term_cnt * (state) + (non_term)))))


/**
 *  \brief  LR(1) parser rule info table accessor
 *
 *  \param  rule_tab  Rule info table
 *  \param  rule_no   Rule number (0-based)
 *
 *  \return Info on rule number \c rule_no
 */
#define sa_rule_table_at(rule_tab, rule_no) \
    (assert((rule_no) < (rule_tab)->rule_cnt), \
     (((rule_tab)->impl)[(rule_no)]))


/*
 * Attribute definitions interface
 */

/** Attribute definitions table */
struct attribute_tab {
    /** Terminal symbols attribute classes */
    const attr_classes_t (*tsyms_classes)[];

    /** Non-terminal symbols attr. classes */
    const attr_classes_t (*ntsyms_classes)[];

    /** Terminal symbols explicit attribute evaluators */
    const attr_eval_descr_refs_t * (*tsyms_expl_evals)[];

    /** Non-terminal symbols explicit attribute evaluators */
    const attr_eval_descr_refs_t * (*ntsyms_expl_evals)[];

    /** Terminal symbols attribute names FSA */
    const attr_name_fsa_t * (*tsyms_names_fsa)[];

    /** Non-terminal symbols attribute names FSA */
    const attr_name_fsa_t * (*ntsyms_names_fsa)[];

};  /* end of struct attribute_tab */


/**
 *  \brief  Get terminal symbol attribute classes
 *
 *  \param  attr_tab  Attribute definitions table
 *  \param  sym       Terminal symbol index (aka lexical item code)
 *
 *  \return Symbol attribute classes
 */
#define sa_attr_table_tsym_classes(attr_tab, sym) \
    ((*((attr_tab)->tsyms_classes))[(sym) - 1])


/**
 *  \brief  Get non-terminal symbol attribute classes
 *
 *  \param  attr_tab  Attribute definitions table
 *  \param  sym       Non-terminal symbol index
 *
 *  \return Symbol attribute classes
 */
#define sa_attr_table_ntsym_classes(attr_tab, sym) \
    ((*((attr_tab)->ntsyms_classes))[(sym)])


/**
 *  \brief  Get terminal symbol explicit attribute evauators
 *
 *  \param  attr_tab  Attribute definitions table
 *  \param  sym       Terminal symbol index (aka lexical item code)
 *
 *  \return Symbol explicit attribute evaluators
 */
#define sa_attr_table_tsym_explicit_evals(attr_tab, sym) \
    ((*((attr_tab)->tsyms_expl_evals))[(sym) - 1])


/**
 *  \brief  Get non-terminal symbol explicit attribute evauators
 *
 *  \param  attr_tab  Attribute definitions table
 *  \param  sym       Non-terminal symbol index
 *
 *  \return Symbol explicit attribute evaluators
 */
#define sa_attr_table_ntsym_explicit_evals(attr_tab, sym) \
    ((*((attr_tab)->ntsyms_expl_evals))[(sym)])


/**
 *  \brief  Get terminal symbol attribute names resolution FSA
 *
 *  \param  attr_tab  Attribute definitions table
 *  \param  sym       Terminal symbol index
 *
 *  \return Symbol attribute names resolution FSA
 */
#define sa_attr_table_tsym_name_fsa(attr_tab, sym) \
    ((*((attr_tab)->tsyms_names_fsa))[(sym) - 1])


/**
 *  \brief  Get non-terminal symbol attribute names resolution FSA
 *
 *  \param  attr_tab  Attribute definitions table
 *  \param  sym       Non-terminal symbol index
 *
 *  \return Symbol attribute names resolution FSA
 */
#define sa_attr_table_ntsym_name_fsa(attr_tab, sym) \
    ((*((attr_tab)->ntsyms_names_fsa))[(sym)])


/** Syntax analyser PDA stack */
struct sa_stack {
    size_t  depth;  /**< Stack current depth                      */
    size_t *impl;   /**< Stack implementation                     */
    size_t  cap;    /**< Stack capacity                           */
    size_t  limit;  /**< Stack capacity limit (0 means unlimited) */
};  /* end of struct sa_stack */


/** Syntax analyser state */
struct sa_state {
    int           accept;        /**< Accept flag       */
    sa_stack_t    pda_stack;     /**< PDA stack         */
    sa_stack_t    reduct_stack;  /**< Word reduction    */
    ptree_node_t *ptree;         /**< Parse tree handle */
};  /* end of struct sa_state */


/** Syntax analyser */
struct syxa {
    const lr1_action_tab_t *action_tab;  /**< LR(1) parser action    table              */
    const lr1_goto_tab_t   *goto_tab;    /**< LR(1) parser goto      table              */
    const lr1_rule_tab_t   *rule_tab;    /**< LR(1) parser rule info table              */
    const attribute_tab_t  *attr_tab;    /**< Attribute definitions  table              */
    lexa_t                  la;          /**< Lexical analyser (input segmenter)        */
    int                     reduct_log;  /**< Non-zero means that reduction is logged   */
    int                     ptree;       /**< Non-zero means that parse tree is created */
    sa_state_t              state;       /**< State                                     */
    sa_status_t             status;      /**< Status                                    */
};  /* end of struct syxa */


/*
 * Syntax analyser interface
 */

/**
 *  \brief  Reduction log enabled check
 *
 *  \param  sa  Syntax analyser
 *
 *  \return Non-zero iff reduction log is enabled
 */
#define sa_reduction_log_enabled(sa) ((sa)->reduct_log)


/**
 *  \brief  Parse tree creation enabled check
 *
 *  \param  sa  Syntax analyser
 *
 *  \return Non-zero iff parse tree creation is enabled
 */
#define sa_ptree_enabled(sa) ((sa)->ptree)


/**
 *  \brief  Add another source buffer
 *
 *  The function passes the buffer to lexical analyser
 *  (i.e. expands to \c la_add_buffer function call).
 *
 *  \param  sa      Syntax analyser
 *  \param  buffer  Source buffer
 */
#define sa_add_buffer(sa, buffer) la_add_buffer(&(sa)->la, (buffer))


/**
 *  \brief  Syntax analyser accepts the input
 *
 *  \param  sa  Syntax analyser
 *
 *  \return Non-zero iff the input is accepted
 */
#define sa_accept(sa) ((sa)->state.accept)


/**
 *  \brief  Provide parse tree
 *
 *  The macro provides the parsed source parse tree
 *  (if the source was accepted by the analyser and
 *  creation of the parse tree is enabled, of course).
 *  In other cases, the returned value meaning is not defined.
 *
 *  Note that the tree MUST be treated as a read-only
 *  structure still in possesion of the parser.
 *  See \ref sa_handover_ptree for a method to overtake the parse tree.
 *
 *  \param  sa  Syntax analyser
 *
 *  \return Parse tree (handled by its root node)
 */
#define sa_ptree_ro(sa) ((sa)->state.ptree)


/**
 *  \brief  Syntax analyser constructor
 *
 *  The \c reduct_stack_* parameters are ignored unless \c reduct_log_enabled
 *  is set.
 *
 *  \param  sa                    Syntax analyser (uninitialised object memory)
 *  \param  la_fsa                Lexical items language FSA
 *  \param  items_total           Lexical items set cardinality (including EoF item)
 *  \param  action_tab            LR(1) action table
 *  \param  goto_tab              LR(1) goto table
 *  \param  rule_tab              LR(1) grammar rules info table
 *  \param  attr_tab              Attribute definitions table
 *  \param  pda_stack_cap_mul     PDA stack initial capacity multiplier
 *  \param  pda_stack_limit       PDA stack depth limit (0 means unlimited)
 *  \param  reduct_log_enabled    Reduction log shall be kept if set
 *  \param  reduct_stack_cap_mul  Reduction stack initial capacity multiplier
 *  \param  reduct_stack_limit    Reduction stack depth limit (0 means unlimited)
 *  \param  ptree_enabled         Parse tree shall be created if set
 *
 *  \return Syntax analyser instance or \c NULL in case of error
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
                  int                     ptree_enabled);


/**
 *  \brief  Syntax analyser destructor
 *
 *  \param  sa  Syntax analyser
 */
void sa_destroy(syxa_t *sa);


/**
 *  \brief  Add another source chunk
 *
 *  The function passes the data chunk to lexical analyser
 *  (i.e. wraps around \c la_add_data function).
 *
 *  \param  sa          Syntax analyser
 *  \param  data        Source chunk
 *  \param  size        Source chunk size
 *  \param  cleanup_fn  Source chunk cleanup function
 *  \param  user_obj    User-specified object (\c cleanup_fn 1st argument)
 *  \param  is_last     Source chunk is last
 *
 *  \retval SA_OK    in case of successful completion
 *  \retval SA_ERROR if buffer could not be created
 */
sa_status_t sa_add_data(syxa_t *sa, char *data, size_t size, buffer_cleanup_fn *cleanup_fn, void *user_obj, int is_last);


/**
 *  \brief  Parse source
 *
 *  The \c sa_parse routine performs greedy parsing of the input,
 *  i.e. it parses until the input is accepted/rejected or exhausted.
 *
 *  \param  sa  Syntax analyser
 *
 *  \retval SA_OK              on success
 *  \retval SA_INPUT_EXHAUSTED if more input data is required to proceed
 *  \retval SA_PARSE_ERROR     if syntax error is detected (input is rejected)
 *  \retval SA_ERROR           on general failure
 */
sa_status_t sa_parse(syxa_t *sa);


/**
 *  \brief  Derivation getter
 *
 *  The function returns dynamically allocated array
 *  containing the rightmost derivation of the parsed word.
 *
 *  Note that the function may either be called incrementally
 *  as well as after the word is accepted.
 *  In case of the incremental use, it provides incremental
 *  parts of the derivation BUT THE SEQUENCE MUST BE REVERSED.
 *
 *  The reason is that the LR parser actually keeps reduction
 *  log (i.e. derivation inversion).
 *  The function inverts the reduction for the caller,
 *  but if called incrementally, the caller must invert
 *  the bits sequence on his own.
 *
 *  \param  sa              Syntax analyser
 *  \param  derivation_len  Length of the derivation
 *
 *  \return The derivation or \c NULL in case of memory error
 */
size_t *sa_derivation(syxa_t *sa, size_t *derivation_len);


/**
 *  \brief  Hand over the parse tree
 *
 *  The function provides the parsed source parse tree
 *  (if the source was accepted by the analyser and
 *  creation of the parse tree is enabled, of course).
 *
 *  By calling this function, the parse tree possession
 *  is passed from the parser to the caller.
 *  Subsequent calls will return \c NULL.
 *  Destruction of the parse tree should be done by
 *  calling \c ptree_destroy.
 *
 *  \param  sa  Syntax analyser
 *
 *  \return Parse tree (handled by its root node)
 */
ptree_node_t *sa_handover_ptree(syxa_t *sa);

#endif /* end of #ifndef CTXFryer__syntax_analyser_h */
