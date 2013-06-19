#ifndef CTXFryer__fsa_h
#define CTXFryer__fsa_h

/**
 *  \brief  FSA
 *
 *  The file is part of CTXFryer C target language libraries.
 *
 *  \date  2012/06/15
 */

#include <stdint.h>
#include <unistd.h>


typedef struct fsa        fsa_t;         /**< FSA                             */
typedef struct fsa_state  fsa_state_t;   /**< FSA state                       */
typedef struct fsa_branch fsa_branch_t;  /**< FSA branch                      */
typedef uint32_t          fsa_id_t;      /**< FSA ID type                     */
typedef unsigned int      fsa_accept_t;  /**< FSA accepted language code type */


/** FSA */
struct fsa {
    const fsa_id_t     id;    /**< ID         */
    const fsa_state_t *root;  /**< Root state */
};  /* end of struct fsa */

/** FSA branch */
struct fsa_branch {
    const fsa_id_t     id;              /**< ID                       */
    const fsa_state_t *target;          /**< Target state             */
    const size_t       interval_cnt;    /**< Character interval count */
    const char         intervals[][2];  /**< Character intervals      */
};  /* end of struct fsa_branch */

/** FSA state */
struct fsa_state {
    const fsa_id_t      id;          /**< ID                           */
    const size_t        accept_cnt;  /**< Accepted language code count */
    const fsa_accept_t *accepts;     /**< Accepted language codes      */
    const size_t        branch_cnt;  /**< Branch count                 */
    const fsa_branch_t *branches[];  /**< Branches                     */
};  /* end of struct fsa_state */


/**
 *  \brief  FSA root state getter
 *
 *  \param  fsa  FSA
 *
 *  \return The FSA root state
 */
#define fsa_root(fsa) ((fsa)->root)

#endif /* end of #ifndef CTXFryer__fsa_h */
