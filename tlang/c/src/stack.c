/**
 *  \brief  Stack
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2013/03/13
 */

#include "stack.h"
#include "objpool.h"
#include "objpack.h"

#include <assert.h>


/** Item pack handle cache size limit */
#define LIST_ITEM_PACK_HANDLE_CACHE_MAX  512


/** Stack item pack handle pool */
static objpool_t stack_item_pack_handle_pool = OBJPOOL_INIT(
    sizeof(stack_item_pack_t), LIST_ITEM_PACK_HANDLE_CACHE_MAX);


static void stack_finalise(void) __attribute__((destructor));


void stack_init(stack_t *stack, objpack_pool_t *item_pool, size_t size_max) {
    assert(NULL != stack);

    stack->item_pool = item_pool;
    stack->item_cnt  = 0;
    stack->size_max  = size_max;
    stack->top_pack  = NULL;
}


void stack_cleanup(stack_t *stack) {
    assert(NULL != stack);

    stack->item_cnt = 0;

    for (;;) {
        stack_item_pack_t *pack = stack->top_pack;

        if (NULL == pack) break;

        stack->top_pack = pack->prev;

        objpack_unref(pack->meta);
        objpool_free(&stack_item_pack_handle_pool, pack);
    }
}


void *stack_push(stack_t *stack) {
    assert(NULL != stack);

    /* Stack overflow */
    if (stack_full(stack)) return NULL;

    size_t cap   = objpack_capacity(stack->item_pool);
    size_t index = stack->item_cnt % cap;

    /** Another item pack is needed */
    if (0 == index) {
        stack_item_pack_t *pack = objpool_alloc(
            &stack_item_pack_handle_pool);

        if (NULL == pack) return NULL;

        pack->items = objpack_alloc(stack->item_pool, cap, &pack->meta);

        if (NULL == pack->items) {
            objpool_free(&stack_item_pack_handle_pool, pack);

            return NULL;
        }

        pack->prev = stack->top_pack;
        stack->top_pack = pack;
    }

    /* Provide new stack top */
    ++stack->item_cnt;

    size_t item_size = objpack_obj_size(stack->item_pool);

    return (char *)stack->top_pack->items + index * item_size;
}


void *stack_top(stack_t *stack) {
    assert(NULL != stack);

    if (stack_empty(stack)) return NULL;

    size_t cap       = objpack_capacity(stack->item_pool);
    size_t index     = (stack->item_cnt - 1) % cap;
    size_t item_size = objpack_obj_size(stack->item_pool);

    return (char *)stack->top_pack->items + index * item_size;
}


void stack_pop(stack_t *stack) {
    assert(NULL != stack);

    /* Stack underflow */
    if (stack_empty(stack)) return;

    size_t item_cnt = --stack->item_cnt;

    /* Top item pack may be released */
    if (0 == item_cnt % objpack_capacity(stack->item_pool)) {
        stack_item_pack_t *pack = stack->top_pack;

        stack->top_pack = pack->prev;

        objpack_unref(pack->meta);
        objpool_free(&stack_item_pack_handle_pool, pack);
    }
}


/**
 *  \brief  Module destructor (GCC only!)
 *
 *  If compiled with gcc this shall be executed after \c main
 *  routine scope is left.
 *  Cleans the item handle pool up.
 */
static void stack_finalise(void) {
    objpool_finalise(&stack_item_pack_handle_pool);
}
