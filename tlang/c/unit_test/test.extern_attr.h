#ifndef test__extern_attr_h
#define test__extern_attr_h

/**
 *  \brief   External attributes functions declarations
 *
 *  \date  2013/04/14
 */

#include "attribute.h"


/**
 *  \brief  Summation
 *
 *  \param[out]  result  Result
 *  \param[in]   l_arg   Left argument
 *  \param[in]   r_arg   Right argument
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR on memory error (result is dynamic)
 */
attr_eval_status_t sum(void **result, void *l_arg, void *r_arg);


/**
 *  \brief  Multiplication
 *
 *  \param[out]  result  Result
 *  \param[in]   l_arg   Left argument
 *  \param[in]   r_arg   Right argument
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR on memory error (result is dynamic)
 */
attr_eval_status_t mul(void **result, void *l_arg, void *r_arg);


/**
 *  \brief  Token-to-double convertor
 *
 *  \param[out]  f1oat  Floating point number
 *  \param[in]   token  Lexical token
 *
 *  \retval ATTR_EVAL_OK    on success
 *  \retval ATTR_EVAL_ERROR on memory error (result is dynamic)
 */
attr_eval_status_t token2num(void **f1oat, void *token);


/**
 *  \brief  Value destructor
 *
 *  \param[in]  val  Value
 */
void destroy_value(void *val);

#endif  /* end of #ifndef test__extern_attr_h */
