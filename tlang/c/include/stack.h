#ifndef CTXFryer__stack_h
#define CTXFryer__stack_h

/**
 *  \brief  Stack
 *
 *  Variable item size stack.
 *
 *  It's build on top of the object pack module which means
 *  that it can reach very high availability (since the space
 *  is allocated in packs).
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2013/03/13
 */

#include "objpack.h"

#include <stddef.h>


/** Absolute maximum of stack items */
#ifdef SIZE_MAX
#define STACK_SIZE_MAX SIZE_MAX
#else
#define STACK_SIZE_MAX ((size_t)-1)
#endif


typedef struct stack stack_t;  /**< Stack handle */

typedef struct stack_item_pack stack_item_pack_t;  /**< Stack item pack */


/** Stack item pack handle */
struct stack_item_pack {
    void              *items;  /**< Packed items        */
    objpack_handle_t  *meta;   /**< Packed items handle */
    stack_item_pack_t *prev;   /**< Previous pack       */
};  /* end of struct stack_item_pack */


/** Stack handle */
struct stack {
    objpack_pool_t    *item_pool;  /**< Item pool        */
    size_t             item_cnt;   /**< Item count       */
    size_t             size_max;   /**< Stack size limit */
    stack_item_pack_t *top_pack;   /**< Top item pack    */
};  /* end of struct stack */


/**
 *  \brief  Count of stacked items
 *
 *  \param  stack  Stack
 */
#define stack_size(stack) ((stack)->item_cnt)


/**
 *  \brief  Check whether the stack is empty
 *
 *  \param  stack  Stack
 */
#define stack_empty(stack) (0 == (stack)->item_cnt)


/**
 *  \brief  Check whether the stack is already full
 *
 *  \param  stack  Stack
 *
 *  \retval non-zero if whole stack capacity was exhausted
 *  \retval 0        otherwise
 */
#define stack_full(stack) ((stack)->item_cnt == (stack)->size_max)


/**
 *  \brief  Initialise stack
 *
 *  The function initialises the stack object memory.
 *  The maximal depth of the stack may be set.
 *
 *  \param  stack      Stack
 *  \param  item_pool  Stack item pack pool
 *  \param  size_max   Stack item limit (e.g. \ref STACK_SIZE_MAX)
 */
void stack_init(stack_t *stack, objpack_pool_t *item_pool, size_t size_max);


/**
 *  \brief  Cleans the stack up
 *
 *  The function removes all items from the stack.
 *  The result is an empty stack ready for use.
 *
 *  Note that the function doesn't provide any item destructor
 *  call-back hook.
 *  If you need to perform items destruction, use \ref stack_top
 *  and \ref stack_pop in a loop (i.e. you need to clean the stack
 *  by hand in that case, sorry... ;-)
 *
 *  \param  stack  Stack
 */
void stack_cleanup(stack_t *stack);


/**
 *  \brief  Push operation
 *
 *  The function interface looks a bit odd since it doesn't take
 *  the item as an argument, but rather returns space allocated
 *  for it (i.e. the stack new top) to avoid unnecessary calls to
 *  \c memcpy or so if the value is actually a primitive type
 *  or generally of small size.
 *
 *  \param  stack  Stack
 *
 *  \return New uninitialised stack top or \c NULL in case of
 *          error or overflow (see \ref stack_full)
 */
void *stack_push(stack_t *stack);


/**
 *  \brief  Stack top
 *
 *  \param  stack  Stack
 *
 *  \return Stack top item or \c NULL if stack is empty
 */
void *stack_top(stack_t *stack);


/**
 *  \brief  Pop operation
 *
 *  The function removes the top item.
 *
 *  Note that the function doesn't provide any item destructor
 *  call-back hook.
 *  If you need to perform the item destruction, use \ref stack_top
 *  before calling \c stack_pop.
 *
 *  \param  stack  Stack
 */
void stack_pop(stack_t *stack);

#endif /* end of #ifndef CTXFryer__stack_h */
