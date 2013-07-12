/**
 *  \brief  FSA 0x0000022d definition
 *
 *  The code is generated; do NOT change it, manually.
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2012/06/15
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

#include "test.fsa_table.h"
#include "test.lexical_items.h"

#include "fsa.h"


/*
 * States and branches tentative definitions
 */

static const fsa_state_t  fsa_0x0000022d_state_0x00000210;
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021e;
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x00000221;
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021c;
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x00000220;
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021f;
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021d;

static const fsa_state_t  fsa_0x0000022d_state_0x00000214;
static const fsa_branch_t fsa_0x0000022d_state_0x00000214_branch_0x00000226;

static const fsa_state_t  fsa_0x0000022d_state_0x00000215;
static const fsa_branch_t fsa_0x0000022d_state_0x00000215_branch_0x00000227;
static const fsa_branch_t fsa_0x0000022d_state_0x00000215_branch_0x00000228;

static const fsa_state_t  fsa_0x0000022d_state_0x00000211;
static const fsa_branch_t fsa_0x0000022d_state_0x00000211_branch_0x00000222;

static const fsa_state_t  fsa_0x0000022d_state_0x00000216;
static const fsa_branch_t fsa_0x0000022d_state_0x00000216_branch_0x00000229;
static const fsa_branch_t fsa_0x0000022d_state_0x00000216_branch_0x0000022a;

static const fsa_state_t  fsa_0x0000022d_state_0x00000212;
static const fsa_branch_t fsa_0x0000022d_state_0x00000212_branch_0x00000224;
static const fsa_branch_t fsa_0x0000022d_state_0x00000212_branch_0x00000223;

static const fsa_state_t  fsa_0x0000022d_state_0x00000213;
static const fsa_branch_t fsa_0x0000022d_state_0x00000213_branch_0x00000225;

static const fsa_state_t  fsa_0x0000022d_state_0x00000217;
static const fsa_branch_t fsa_0x0000022d_state_0x00000217_branch_0x0000022b;

static const fsa_state_t  fsa_0x0000022d_state_0x0000021b;

static const fsa_state_t  fsa_0x0000022d_state_0x0000021a;

static const fsa_state_t  fsa_0x0000022d_state_0x00000219;

static const fsa_state_t  fsa_0x0000022d_state_0x00000218;
static const fsa_branch_t fsa_0x0000022d_state_0x00000218_branch_0x0000022c;


/**
 *  \brief  FSA 0x0000022d
 *
 *  U(U(U(U("[-+]?\d+(\.\d+([eE][-+]?\d+)?)?" alternative 0 minimal, "\+" alternative 0 minimal), "\*" alternative 0 minimal), "\)" alternative 0 minimal), "\(" alternative 0 minimal) minimal
 */
const fsa_t test_fsa = {
    .id          = 0x0000022d,
    .root        = &fsa_0x0000022d_state_0x00000210
}; /* end of FSA 0x0000022d definition */


/*
 * States and branches definitions
 */

/**
 *  \brief  FSA 0x0000022d state 0x00000210
 *
 *  U(U(U(U("[-+]?\d+(\.\d+([eE][-+]?\d+)?)?" alternative 0 minimal, "\+" alternative 0 minimal), "\*" alternative 0 minimal), "\)" alternative 0 minimal), "\(" alternative 0 minimal) root
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000210 = {
    .id          = 0x00000210,
    .accept_cnt  = 0,
    .accepts     = NULL,
    .branch_cnt  = 6,
    .branches    = {
        &fsa_0x0000022d_state_0x00000210_branch_0x0000021e,
        &fsa_0x0000022d_state_0x00000210_branch_0x00000221,
        &fsa_0x0000022d_state_0x00000210_branch_0x0000021c,
        &fsa_0x0000022d_state_0x00000210_branch_0x00000220,
        &fsa_0x0000022d_state_0x00000210_branch_0x0000021f,
        &fsa_0x0000022d_state_0x00000210_branch_0x0000021d,
    }
}; /* end of FSA state 0x00000210 */

/**
 *  \brief  FSA 0x0000022d state 0x00000210 branch 0x0000021e
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021e = {
    .id           = 0x0000021e,
    .target       = &fsa_0x0000022d_state_0x00000214,
    .interval_cnt = 1,
    .intervals    = {
        { '-', '-' },
    }
}; /* end of FSA 0x0000022d state 0x00000210 branch 0x0000021e */

/**
 *  \brief  FSA 0x0000022d state 0x00000210 branch 0x00000221
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x00000221 = {
    .id           = 0x00000221,
    .target       = &fsa_0x0000022d_state_0x0000021b,
    .interval_cnt = 1,
    .intervals    = {
        { '(', '(' },
    }
}; /* end of FSA 0x0000022d state 0x00000210 branch 0x00000221 */

/**
 *  \brief  FSA 0x0000022d state 0x00000210 branch 0x0000021c
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021c = {
    .id           = 0x0000021c,
    .target       = &fsa_0x0000022d_state_0x00000215,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000210 branch 0x0000021c */

/**
 *  \brief  FSA 0x0000022d state 0x00000210 branch 0x00000220
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x00000220 = {
    .id           = 0x00000220,
    .target       = &fsa_0x0000022d_state_0x0000021a,
    .interval_cnt = 1,
    .intervals    = {
        { ')', ')' },
    }
}; /* end of FSA 0x0000022d state 0x00000210 branch 0x00000220 */

/**
 *  \brief  FSA 0x0000022d state 0x00000210 branch 0x0000021f
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021f = {
    .id           = 0x0000021f,
    .target       = &fsa_0x0000022d_state_0x00000219,
    .interval_cnt = 1,
    .intervals    = {
        { '*', '*' },
    }
}; /* end of FSA 0x0000022d state 0x00000210 branch 0x0000021f */

/**
 *  \brief  FSA 0x0000022d state 0x00000210 branch 0x0000021d
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000210_branch_0x0000021d = {
    .id           = 0x0000021d,
    .target       = &fsa_0x0000022d_state_0x00000218,
    .interval_cnt = 1,
    .intervals    = {
        { '+', '+' },
    }
}; /* end of FSA 0x0000022d state 0x00000210 branch 0x0000021d */

/**
 *  \brief  FSA 0x0000022d state 0x00000214
 *
 *  "[-+]?\d+(\.\d+([eE][-+]?\d+)?)?" alternative 0 symbol set iteration 0
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000214 = {
    .id          = 0x00000214,
    .accept_cnt  = 0,
    .accepts     = NULL,
    .branch_cnt  = 1,
    .branches    = {
        &fsa_0x0000022d_state_0x00000214_branch_0x00000226,
    }
}; /* end of FSA state 0x00000214 */

/**
 *  \brief  FSA 0x0000022d state 0x00000214 branch 0x00000226
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000214_branch_0x00000226 = {
    .id           = 0x00000226,
    .target       = &fsa_0x0000022d_state_0x00000215,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000214 branch 0x00000226 */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x00000215 */
static const fsa_accept_t fsa_0x0000022d_state_0x00000215_accepts[] = { LEXI_number };

/**
 *  \brief  FSA 0x0000022d state 0x00000215
 *
 *  "[-+]?\d+(\.\d+([eE][-+]?\d+)?)?" alternative 0 symbol set iteration 1
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000215 = {
    .id          = 0x00000215,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x00000215_accepts,
    .branch_cnt  = 2,
    .branches    = {
        &fsa_0x0000022d_state_0x00000215_branch_0x00000227,
        &fsa_0x0000022d_state_0x00000215_branch_0x00000228,
    }
}; /* end of FSA state 0x00000215 */

/**
 *  \brief  FSA 0x0000022d state 0x00000215 branch 0x00000227
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000215_branch_0x00000227 = {
    .id           = 0x00000227,
    .target       = &fsa_0x0000022d_state_0x00000215,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000215 branch 0x00000227 */

/**
 *  \brief  FSA 0x0000022d state 0x00000215 branch 0x00000228
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000215_branch_0x00000228 = {
    .id           = 0x00000228,
    .target       = &fsa_0x0000022d_state_0x00000211,
    .interval_cnt = 1,
    .intervals    = {
        { '.', '.' },
    }
}; /* end of FSA 0x0000022d state 0x00000215 branch 0x00000228 */

/**
 *  \brief  FSA 0x0000022d state 0x00000211
 *
 *  "\.\d+([eE][-+]?\d+)?" alternative 0 symbol set iteration 0
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000211 = {
    .id          = 0x00000211,
    .accept_cnt  = 0,
    .accepts     = NULL,
    .branch_cnt  = 1,
    .branches    = {
        &fsa_0x0000022d_state_0x00000211_branch_0x00000222,
    }
}; /* end of FSA state 0x00000211 */

/**
 *  \brief  FSA 0x0000022d state 0x00000211 branch 0x00000222
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000211_branch_0x00000222 = {
    .id           = 0x00000222,
    .target       = &fsa_0x0000022d_state_0x00000216,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000211 branch 0x00000222 */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x00000216 */
static const fsa_accept_t fsa_0x0000022d_state_0x00000216_accepts[] = { LEXI_number };

/**
 *  \brief  FSA 0x0000022d state 0x00000216
 *
 *  "\.\d+([eE][-+]?\d+)?" alternative 0 symbol set iteration 1
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000216 = {
    .id          = 0x00000216,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x00000216_accepts,
    .branch_cnt  = 2,
    .branches    = {
        &fsa_0x0000022d_state_0x00000216_branch_0x00000229,
        &fsa_0x0000022d_state_0x00000216_branch_0x0000022a,
    }
}; /* end of FSA state 0x00000216 */

/**
 *  \brief  FSA 0x0000022d state 0x00000216 branch 0x00000229
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000216_branch_0x00000229 = {
    .id           = 0x00000229,
    .target       = &fsa_0x0000022d_state_0x00000212,
    .interval_cnt = 2,
    .intervals    = {
        { 'E', 'E' },
        { 'e', 'e' },
    }
}; /* end of FSA 0x0000022d state 0x00000216 branch 0x00000229 */

/**
 *  \brief  FSA 0x0000022d state 0x00000216 branch 0x0000022a
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000216_branch_0x0000022a = {
    .id           = 0x0000022a,
    .target       = &fsa_0x0000022d_state_0x00000216,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000216 branch 0x0000022a */

/**
 *  \brief  FSA 0x0000022d state 0x00000212
 *
 *  "[eE][-+]?\d+" alternative 0 symbol set iteration 0
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000212 = {
    .id          = 0x00000212,
    .accept_cnt  = 0,
    .accepts     = NULL,
    .branch_cnt  = 2,
    .branches    = {
        &fsa_0x0000022d_state_0x00000212_branch_0x00000224,
        &fsa_0x0000022d_state_0x00000212_branch_0x00000223,
    }
}; /* end of FSA state 0x00000212 */

/**
 *  \brief  FSA 0x0000022d state 0x00000212 branch 0x00000224
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000212_branch_0x00000224 = {
    .id           = 0x00000224,
    .target       = &fsa_0x0000022d_state_0x00000213,
    .interval_cnt = 2,
    .intervals    = {
        { '+', '+' },
        { '-', '-' },
    }
}; /* end of FSA 0x0000022d state 0x00000212 branch 0x00000224 */

/**
 *  \brief  FSA 0x0000022d state 0x00000212 branch 0x00000223
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000212_branch_0x00000223 = {
    .id           = 0x00000223,
    .target       = &fsa_0x0000022d_state_0x00000217,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000212 branch 0x00000223 */

/**
 *  \brief  FSA 0x0000022d state 0x00000213
 *
 *  "[eE][-+]?\d+" alternative 0 symbol set iteration 1
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000213 = {
    .id          = 0x00000213,
    .accept_cnt  = 0,
    .accepts     = NULL,
    .branch_cnt  = 1,
    .branches    = {
        &fsa_0x0000022d_state_0x00000213_branch_0x00000225,
    }
}; /* end of FSA state 0x00000213 */

/**
 *  \brief  FSA 0x0000022d state 0x00000213 branch 0x00000225
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000213_branch_0x00000225 = {
    .id           = 0x00000225,
    .target       = &fsa_0x0000022d_state_0x00000217,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000213 branch 0x00000225 */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x00000217 */
static const fsa_accept_t fsa_0x0000022d_state_0x00000217_accepts[] = { LEXI_number };

/**
 *  \brief  FSA 0x0000022d state 0x00000217
 *
 *  "[eE][-+]?\d+" alternative 0 symbol set iteration 2
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000217 = {
    .id          = 0x00000217,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x00000217_accepts,
    .branch_cnt  = 1,
    .branches    = {
        &fsa_0x0000022d_state_0x00000217_branch_0x0000022b,
    }
}; /* end of FSA state 0x00000217 */

/**
 *  \brief  FSA 0x0000022d state 0x00000217 branch 0x0000022b
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000217_branch_0x0000022b = {
    .id           = 0x0000022b,
    .target       = &fsa_0x0000022d_state_0x00000217,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000217 branch 0x0000022b */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x0000021b */
static const fsa_accept_t fsa_0x0000022d_state_0x0000021b_accepts[] = { LEXI_lpar };

/**
 *  \brief  FSA 0x0000022d state 0x0000021b
 *
 *  "\(" alternative 0 symbol set iteration 0
 */
static const fsa_state_t fsa_0x0000022d_state_0x0000021b = {
    .id          = 0x0000021b,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x0000021b_accepts,
    .branch_cnt  = 0,
    .branches    = {
    }
}; /* end of FSA state 0x0000021b */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x0000021a */
static const fsa_accept_t fsa_0x0000022d_state_0x0000021a_accepts[] = { LEXI_rpar };

/**
 *  \brief  FSA 0x0000022d state 0x0000021a
 *
 *  "\)" alternative 0 symbol set iteration 0
 */
static const fsa_state_t fsa_0x0000022d_state_0x0000021a = {
    .id          = 0x0000021a,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x0000021a_accepts,
    .branch_cnt  = 0,
    .branches    = {
    }
}; /* end of FSA state 0x0000021a */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x00000219 */
static const fsa_accept_t fsa_0x0000022d_state_0x00000219_accepts[] = { LEXI_mul };

/**
 *  \brief  FSA 0x0000022d state 0x00000219
 *
 *  "\*" alternative 0 symbol set iteration 0
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000219 = {
    .id          = 0x00000219,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x00000219_accepts,
    .branch_cnt  = 0,
    .branches    = {
    }
}; /* end of FSA state 0x00000219 */

/** \brief Lexical items accepted by FSA 0x0000022d state 0x00000218 */
static const fsa_accept_t fsa_0x0000022d_state_0x00000218_accepts[] = { LEXI_add };

/**
 *  \brief  FSA 0x0000022d state 0x00000218
 *
 *  U("[-+]?\d+(\.\d+([eE][-+]?\d+)?)?" alternative 0 symbol set iteration 0, "\+" alternative 0 symbol set iteration 0)
 */
static const fsa_state_t fsa_0x0000022d_state_0x00000218 = {
    .id          = 0x00000218,
    .accept_cnt  = 1,
    .accepts     = fsa_0x0000022d_state_0x00000218_accepts,
    .branch_cnt  = 1,
    .branches    = {
        &fsa_0x0000022d_state_0x00000218_branch_0x0000022c,
    }
}; /* end of FSA state 0x00000218 */

/**
 *  \brief  FSA 0x0000022d state 0x00000218 branch 0x0000022c
 */
static const fsa_branch_t fsa_0x0000022d_state_0x00000218_branch_0x0000022c = {
    .id           = 0x0000022c,
    .target       = &fsa_0x0000022d_state_0x00000215,
    .interval_cnt = 1,
    .intervals    = {
        { '0', '9' },
    }
}; /* end of FSA 0x0000022d state 0x00000218 branch 0x0000022c */
