/**
 *  \brief   External attributes functions definitions
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2013/04/14
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2013 Vaclav Krpec
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

#include "test.extern_attr.h"

#include "test.h"
#include "token.h"

#include <assert.h>
#include <stdlib.h>


/** \cond */
extern int log_level;
#define LOG_LEVEL log_level
/** \endcond */


attr_eval_status_t sum(void **result, void *l_arg, void *r_arg) {
    assert(NULL != result);
    assert(NULL != l_arg);
    assert(NULL != r_arg);

    /* Allocate result memory */
    *result = malloc(sizeof(double));

    if (NULL == *result) return ATTR_EVAL_ERROR;

    /* Compute result */
    *((double *)*result) = *(double *)l_arg + *(double *)r_arg;

    INFO("%g + %g == %g", *(double *)l_arg, *(double *)r_arg, *((double *)*result));

    return ATTR_EVAL_OK;
}


attr_eval_status_t mul(void **result, void *l_arg, void *r_arg) {
    assert(NULL != result);
    assert(NULL != l_arg);
    assert(NULL != r_arg);

    /* Allocate result memory */
    *result = malloc(sizeof(double));

    if (NULL == *result) return ATTR_EVAL_ERROR;

    /* Compute result */
    *((double *)*result) = *(double *)l_arg * *(double *)r_arg;

    INFO("%g * %g == %g", *(double *)l_arg, *(double *)r_arg, *((double *)*result));

    return ATTR_EVAL_OK;
}


attr_eval_status_t token2num(void **f1oat, void *token) {
    assert(NULL != f1oat);
    assert(NULL != token);

    /* Create token C-string (copy) */
    char *c_str = token_string((token_t *)token);

    if (NULL == c_str) return ATTR_EVAL_ERROR;

    /* Allocate numeric representation memory */
    *f1oat = malloc(sizeof(double));

    if (NULL == *f1oat) {
        free(c_str);  /* release token C-string */

        return ATTR_EVAL_ERROR;
    }

    /* Convert string to float */
    *((double *)*f1oat) = atof(c_str);

    /* Release token C-string */
    free(c_str);

    INFO("Created value %g", *(double *)*f1oat);

    return ATTR_EVAL_OK;
}


void destroy_value(void *val) {
    assert(NULL != val);

    INFO("Destroying value %g", *(double *)val);

    /* Release value memory */
    free(val);
}
