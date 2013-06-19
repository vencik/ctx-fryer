#ifndef CTXFryer__lexical_analyser_h
#define CTXFryer__lexical_analyser_h

/**
 *  \brief  Lexical analyser
 *
 *  Lexical analyser uses FSA (Finite State Automaton) to segment
 *  the input stream (typically a character stream) into a stream
 *  of lexical items.
 *  Lexical item is a word from a regular language;
 *  the FSA is constructed as union of FSA for each particular
 *  lexical item language.
 *
 *  The lexical analyser simply uses the FSA to parse the input
 *  stream.
 *  Its approach is greedy, i.e. it parses as much of the input
 *  as possible.
 *  When further parsing is impossible, the outcome depends on
 *  whether an accepting state was visited during the parsing.
 *  If there were no accepting states on the parse path,
 *  the automaton reports an unexpected string in the input.
 *  If there was at least one accepting state, the automaton
 *  reports match of all respective lexical items prioritised
 *  by descending order of parse path length (greedy approach).
 *
 *  When the automaton reaches end of the input, special EOF
 *  lexical item is provided.
 *
 *  As soon as lexical item is found on the input head,
 *  the analyser waits until the user commands it to read it
 *  (specifying which one in case more than one alternative
 *  exist).
 *  Until then, the analyser state doesn't change.
 *
 *  Also, support for incremental parsing is implemented.
 *  That means that if the input is incomplete and the automaton
 *  reads it all, it reports the input exhaustion and keeps
 *  its current state.
 *  As soon as more input is available, the segmentation process
 *  shall continue as if it was never interrupted.
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2012/06/19
 */

#include "fsa.h"
#include "buffer.h"

#include <unistd.h>


#define LEXIG_INVALID (-1)  /**< Invalidated general lexical item code */
#define LEXIG_EOF       0   /**< End-Of-File general lexical item code */


/** Lexical analyser status codes */
typedef enum {
    LA_OK = 0,           /**< Success                      */
    LA_INPUT_EXHAUSTED,  /**< Input exhausted              */
    LA_INPUT_INVALID,    /**< Unexpected sequence in input */
    LA_ERROR,            /**< General error                */
} la_status_t;  /* end of typedef enum */


/** Lexical item description */
struct la_item {
    int       code;         /**< Item code                                          */
    size_t    offset;       /**< Item offset in the input                           */
    size_t    length;       /**< Item length                                        */
    size_t    line;         /**< Item line   number (starting by 1)                 */
    size_t    column;       /**< Item column number (starting by 1)                 */
    size_t    next_line;    /**< Next item line   number (starting by 1)            */
    size_t    next_column;  /**< Next item column number (starting by 1)            */
    buffer_t *buff_1st;     /**< Buffer holding the 1st character of the item token */
    size_t    buff_off;     /**< Offset of the 1st character in the above buffer    */
};  /* end of struct la_item */

typedef struct la_item la_item_t;  /**< Lexical item description */


/*
 * Lexical item attribute accessors
 */

/**
 *  \brief  Lexical item code
 *
 *  \param  item  Lexical item
 *
 *  \return Lexical item code
 */
#define la_item_code(item) ((item)->code)


/**
 *  \brief  Lexical item token offset
 *
 *  The macro expands to 0-based offset
 *  of the lexical item token 1st octet in source.
 *
 *  \param  item  Lexical item
 *
 *  \return Lexical item token offset (from input begin, 0-based)
 */
#define la_item_token_offset(item) ((item)->offset)


/**
 *  \brief  Lexical item token length
 *
 *  The macro expands to length of the lexical
 *  item token in the source octet stream.
 *
 *  \param  item  Lexical item
 *
 *  \return Lexical item token length
 */
#define la_item_token_length(item) ((item)->length)


/**
 *  \brief  Lexical item token line number
 *
 *  The macro expands to number of LF-ended source line
 *  on which the item token begins.
 *  Lines are numbered starting by 1.
 *
 *  Together with \ref la_item_token_column, the macra
 *  may be used for revealing user-friendly position
 *  of the lexical item in line-organised source file.
 *
 *  \param  item  Lexical item
 *
 *  \return Line number
 */
#define la_item_token_line(item) ((item)->line)


/**
 *  \brief  Lexical item token column
 *
 *  The macro expands to number of column relative to
 *  the begining of the token line.
 *  Columns are numbered starting by 1.
 *
 *  Together with \ref la_item_token_line, the macra
 *  may be used for revealing user-friendly position
 *  of the lexical item in line-organised source file.
 *
 *  \param  item  Lexical item
 *
 *  \return Column number
 */
#define la_item_token_column(item) ((item)->column)


typedef struct la_state la_state_t;  /**< Lexical analyser state */
typedef struct lexa     lexa_t;      /**< Lexical analyser       */


/** Lexical analyser state */
struct la_state {
    const fsa_state_t *fsa_state;       /**< Current item lang. FSA state          */
    int                seg_int;         /**< Segmentation interrupt flag           */
    la_item_t         *item_list;       /**< Item list                             */
    size_t             item_cnt;        /**< Item list length                      */
    size_t            *item_pos;        /**< Positions of items in list            */
    size_t             token_offset;    /**< Current offset of the input head      */
    size_t             token_length;    /**< Current length of the input head      */
    size_t             token_line;      /**< Line   number of head (starting by 1) */
    size_t             token_column;    /**< Column number of head (starting by 1) */
    buffer_t          *token_buff;      /**< Buffer hosting the input head         */
    size_t             token_buff_off;  /**< Offset of the head in the above       */
    size_t             line;            /**< Current line   number (starting by 1) */
    size_t             column;          /**< Current column number (starting by 1) */
    size_t             buffer_offset;   /**< Offset in the current buffer          */
};  /* end of struct la_state */


/** Lexical analyser */
struct lexa {
    const fsa_t *fsa;          /**< Item lang. FSA       */
    buffer_t    *buffer;       /**< Current buffer       */
    buffer_t    *buff_last;    /**< Last buffer in seq.  */
    la_state_t   state;        /**< Current state        */
    size_t       items_total;  /**< Item set cardinality */
    la_status_t  status;       /**< Current status       */
};  /* end of struct lexa */


/*
 * Lexical analyser interface
 */

/**
 *  \brief  Current lexical analyser status
 *
 *  \param  la  Lexical analyser
 *
 *  \return Lexical analyser status
 */
#define la_status(la) ((la)->status)


/**
 *  \brief  Lexical analyser constructor
 *
 *  \param  la           Lexical analyser (uninitialised object memory)
 *  \param  fsa          Lexical items language FSA
 *  \param  items_total  Lexical items set cardinality (including EoF item)
 *
 *  \return Lexical analyser instance
 */
lexa_t *la_create(lexa_t *la, const fsa_t *fsa, size_t items_total);


/**
 *  \brief  Lexical analyser destructor
 *
 *  \param  la  Lexical analyser
 */
void la_destroy(lexa_t *la);


/**
 *  \brief  Add another source chunk
 *
 *  The function creates another buffer for the chunk
 *  and calls \ref la_add_buffer.
 *
 *  \param  la          Lexical analyser
 *  \param  data        Source chunk
 *  \param  size        Source chunk size
 *  \param  cleanup_fn  Source chunk cleanup function
 *  \param  user_obj    User-specified object (\c cleanup_fn 1st argument)
 *  \param  is_last     Source chunk is last
 *
 *  \retval LA_OK    in case of successful completion
 *  \retval LA_ERROR if buffer could not be created
 */
la_status_t la_add_data(lexa_t *la, char *data, size_t size, buffer_cleanup_fn *cleanup_fn, void *user_obj, int is_last);


/**
 *  \brief  Add another source buffer
 *
 *  The function appends another source buffer
 *  to the buffer sequence end.
 *
 *  \param  la      Lexical analyser
 *  \param  buffer  Source buffer
 */
void la_add_buffer(lexa_t *la, buffer_t *buffer);


/**
 *  \brief  Get next lexical items (alternative)
 *
 *  The function returns alternative list of lexical items
 *  that may be read from the input head.
 *
 *  Each of the items therefore constitute a prefix of the
 *  input that is accepted by the items union FSA.
 *  For each item code, there's at most one item in the list
 *  and it's the longest matching prefix of the input.
 *
 *  The items appear in the list in the order of accepting states
 *  trespass, meaning that longer items are pushed to back.
 *  However, it is generally possible that longer item appears
 *  before shorter one.
 *  This happens in case of re-definition, consider the following
 *  example:
 *    item1 = /a+/
 *    item2 = /aa/
 *  Now, if the input begins with "aaa", then item1 is pushed after
 *  checking the 1st character, item2 is pushed after the 2nd and
 *  after the 3rd, item1 matches again and the entry is updated
 *  (redefined).
 *  Now, item1 is enlisted before item2, though it's 3 characters
 *  long while item2 is just 2 characters long.
 *
 *  The function is greedy; it won't report \c LA_OK status
 *  unless no more lexical item alternatives might be parsed.
 *
 *  Also take a look at \ref la_get_item (it's used for checking
 *  whether an item appears on the input head).
 *
 *  The list is cached until any of the items is read
 *  by the \ref la_read_item function (i.e. the function may be
 *  called multiple times and it will provide the same
 *  result with minimal overhead, until \ref la_read_item is called).
 *
 *  As soon as the input segmentation is over (reading reaches
 *  end of file), the function provides the \ref LEXIG_EOF as
 *  a single item.
 *
 *  An item is supposed to be copied by user using \ref la_item_copy.
 *  This is necessary to obtain the item token (since it may require
 *  data copying).
 *  However, to get trivial info (item code, position etc), the item
 *  doesn't really need copying.
 *  List of getters that don't require prior copying follows:
 *  \ref la_item_code
 *  \ref la_item_token_offset
 *  \ref la_item_token_length
 *  \ref la_item_token_line
 *  \ref la_item_token_column
 *
 *  \param  la        Lexical analyser
 *  \param  items     Lexical items list (optional)
 *  \param  item_cnt  Lexical items count (optional)
 *
 *  \retval LA_OK              at least one lexical item is available
 *  \retval LA_INPUT_EXHAUSTED if input is exhausted
 *  \retval LA_INPUT_INVALID   if no valid lexical item is recognised
 *  \retval LA_ERROR           in case of an error
 */
la_status_t la_get_items(lexa_t *la, const la_item_t **items, size_t *item_cnt);


/**
 *  \brief  Get next lexical item by code
 *
 *  The function attempts to obtain lexical item requested.
 *  It may perform less operations than \ref la_get_items
 *  (but only if different lexical items may share common
 *  token prefix).
 *
 *  If the item can't be provided, \c NULL is returned.
 *  Note that this may either signalise that the item
 *  isn't on the input head or that the input is exhausted
 *  or even general parser error.
 *  Use \ref la_status to check what happened.
 *
 *  \param  la         Lexical analyser
 *  \param  item_code  Lexical item code
 *
 *  \return Lexical item or \c NULL if it can't be provided
 */
const la_item_t *la_get_item(lexa_t *la, int item_code);


/**
 *  \brief  Read current lexical item
 *
 *  The function causes shift in input by length
 *  of current lexical item specified by its code
 *  in the item list.
 *
 *  Note that this means that \ref la_get_items or \ref la_get_item
 *  MUST be successfully called at least once before this function.
 *
 *  Also note that the function invalidates all currently valid
 *  lexical items in the analyser item list (copies are safe).
 *
 *  \param  la         Lexical analyser
 *  \param  item_code  Lexical item code
 *
 *  \retval LA_OK    if lexical item was read
 *  \retval LA_ERROR if lexical item of the code isn't available
 */
la_status_t la_read_item(lexa_t *la, int item_code);


/**
 *  \brief  Get lexical item token
 *
 *  The function provides read-only access to lexical item
 *  token.
 *
 *  IMPORTANT NOTE:
 *  The function MUSTN'T be called directly on the \c const
 *  items provided by \ref la_get_items or \ref la_get_item functions.
 *  Always make yourself a copy using \ref la_item_copy, first.
 *
 *  The token is NOT a C-string (i.e. it's NOT generally
 *  zero-terminated), it's either a direct address of
 *  the item 1st character in the input buffer or,
 *  if the item spans multiple buffers, the function
 *  actually creates a new buffer to provide a concatenation.
 *  in this case, the previously used buffers are unreferenced
 *  and the new one is cached for future calls of the function
 *  (so that the copying is amortised).
 *
 *  Note that the above means that the function always returns
 *  the same value and also, the value stays valid as long as
 *  the lexical item remains valid.
 *  However, storing the token address apart the item object
 *  is not recommended (as keeping logical consistency of
 *  such fragmented data model is highly difficult).
 *  You should call this getter if you need the token.
 *
 *  Justification of not providing C-string is in 2 points:
 *  1/ it would require token copying
 *     (mostly unnecessary if buffers are large enough)
 *  2/ it's even impossible if the token is allowed
 *     to contain the null character itself
 *
 *  \param[in]   item    Lexical item
 *  \param[out]  length  Length of the lexical item token
 *
 *  \return Address of complete item token octet sequence
 *          or \c NULL in case of memory error
 */
const char *la_item_token(la_item_t *item, size_t *length);


/**
 *  \brief  Copy lexical item
 *
 *  The function creates lexical item copy.
 *  It's mainly intended for obtaining lexical item
 *  from the analyser before it is read via
 *  \ref la_read_item (if it's necessary to store it
 *  and/or take its token).
 *
 *  Note that the function doesn't copy any data;
 *  it only initialises the destination argument
 *  by the source and reference the item buffer(s).
 *  The overhead is therefore quite negligable.
 *
 *  \param  dest  Copy
 *  \param  src   Original
 */
void la_item_copy(la_item_t *dest, const la_item_t *src);


/**
 *  \brief  Lexical item destructor
 *
 *  The function should be called on all copied lexical
 *  items as soon as they are no longer used.
 *  Note that failing to do so may (and will) cause memory
 *  leaks of source buffers referenced by such incorrectly
 *  abandoned items.
 *  Also note that you MUSTN'T call the destructor on
 *  \c const items provided by \ref la_get_items or \ref la_get_item.
 *  These are under control of the lexical analyser.
 *
 *  \param  item  Lexical item
 */
void la_item_destroy(la_item_t *item);

#endif /* end of #ifndef CTXFryer__lexical_analyser_h */
