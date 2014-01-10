/**
 *  \brief  Simple calculator example
 *
 *  The file is part of CTX Fryer.
 *
 *  \date  2013/08/21
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2013 Vaclav Krpec
 *
 *  CTX Fryer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "parser/C/parser.h"

#include <ctx-fryer-c-rt/buffer.h>
#include <ctx-fryer-c-rt/fsa.h>
#include <ctx-fryer-c-rt/lexical_analyser.h>
#include <ctx-fryer-c-rt/syntax_analyser.h>

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/**
 *  \brief  Parse input
 *
 *  \param  syxa  Syntax analyser
 *
 *  \retval 0 on success
 *  \retval 1 on error
 */
static int parse(syxa_t *syxa) {
    sa_status_t parser_status = sa_parse(syxa);

    switch (parser_status) {
        case SA_OK:
            return 0;

        case SA_INPUT_EXHAUSTED:
            fprintf(stderr, "Input incomplete\n");

            break;

        case SA_SYNTAX_ERROR:
            /* TODO: Report position */
            fprintf(stderr, "Syntax error\n");

            break;

        case SA_ERROR:
            fprintf(stderr, "Parser failure!  Please report this bug.");

            break;
    }

    return 1;
}


/**
 *  \brief  Evaluate parsed expression
 *
 *  \param  syxa  Syntax analyser
 *
 *  \retval 0 on success
 *  \retval 1 on error
 */
static int evaluate(syxa_t *syxa) {
    ptree_node_t *ptree_root = sa_ptree_ro(syxa);

    void *value = "value";

    attr_eval_status_t eval_status =
        ptree_node_attrs_eval(ptree_root, &value, NULL);

    if (ATTR_EVAL_OK != eval_status) {
        fprintf(stderr, "Failed to evaluate the expression: %d", eval_status);

        return 1;
    }

    assert(NULL != value);

    fprintf(stdout, "%g\n", *(const double *)value);

    return 0;
}


/**
 *  \brief  Interpret input line and print result
 *
 *  \param  line      Input line
 *  \param  line_len  Length of the input line
 *
 *  \retval 0 on successful interpretation
 *  \retval 1 on error
 */
static int interpret(char *line, size_t line_len) {
    assert(NULL != line);

    /* Create line syntax analyser */
    syxa_t syxa;

    if (NULL == parser_create(&syxa,
        1,  /* PDA stack initial capacity multiplier */
        0,  /* PDA stack is unlimited */
        0,  /* Reduction log shall not be kept */
        1,  /* Reduction stack initial capacity multiplier */
        0,  /* Reduction stack is unlimited */
        1)  /* Parse tree shall be created */
        )
    {
        fprintf(stderr, "Failed to create parser\n");

        return 1;
    }

    int status = 1;  /* pessimistic assumption */

    /* Push line to parser (no chunk cleaning, last chunk) */
    sa_status_t data_status = sa_add_data(&syxa, line, line_len, NULL, NULL, 1);

    /* Failed to add data */
    if (SA_OK != data_status) {
        fprintf(stderr, "Failed to push input line to the parser: %d\n", data_status);

        status = 1;
    }

    /* Parse line and evaluate the expression */
    else {
        status = parse(&syxa);

        if (0 == status)
            status = evaluate(&syxa);
    }

    /* Destroy syntax analyser */
    sa_destroy(&syxa);

    return status;
}


/** Main routine */
int main(int argc, const char * const argv[]) {
    int exit_code = 0;

    /* Input lines interpretation */
    char   *line = NULL;
    size_t  line_size;
    ssize_t line_len;

    while (-1 != (line_len = getline(&line, &line_size, stdin))) {
        /* Remove EoL */
        if (line_len && '\n' == line[line_len - 1]) line[--line_len] = '\0';
        if (line_len && '\r' == line[line_len - 1]) line[--line_len] = '\0';

        exit_code += 0 == interpret(line, line_len) ? 0 : 1;

        free(line);
        line = NULL;
    }

    exit(exit_code);
}
