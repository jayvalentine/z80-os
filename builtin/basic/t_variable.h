#ifndef _T_VARIABLE_H
#define _T_VARIABLE_H

#include <stdint.h>

#include "t_defs.h"
#include "t_numeric.h"

#include "errors.h"

#define VARNAME_SIZE 4
#define VARNAME_BUF_SIZE (VARNAME_SIZE+1)

/* Maximum number of bytes needed to hold a variable token.
 * Name + token + size + null
 * 4    + 1     + 1    + 1    = 7
 */
#define VAR_TOK_BUF_SIZE (VARNAME_SIZE+3)

/* Gets a pointer to the name field of a variable token
 */
#define VARIABLE_GET(_toks) ((const char *)(_toks+2))

/* Size of a register token. */
#define REGISTER_SIZE 2

/* varcmp
 *
 * Purpose:
 *     Compare two variable tokens.
 * 
 * Parameters:
 *     left:  variable token pointer
 *     right: variable token pointer
 * 
 * Returns:
 *     0 if equal, 1 otherwise.
 */
int varcmp(const tok_t * left, const tok_t * right);

/* t_variable_parse
 *
 * Purpose:
 *     Parse a variable from the input stream.
 * 
 * Parameters:
 *     dst_ptr:   Reference to pointer in destination stream.
 *     input_ptr: Reference to pointer in input stream.
 * 
 * Returns:
 *     1 if successful, 0 otherwise.
 */
int t_variable_parse(tok_t ** dst_ptr, const char ** input_ptr);

/* t_variable_list
 *
 * Purpose:
 *     Print an variable token.
 * 
 * Parameters:
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Pointer to token after the variable.
 */
const tok_t * t_variable_list(const tok_t * toks);

/* t_variable_get_ptr
 *
 * Purpose:
 *     Returns a pointer to the value of a variable
 *     (scalar or array element) for access.
 * 
 * Parameters:
 *     toks:      Token stream
 *     next_toks: Populated with pointer to next tok after the variable access.
 *     ptr:       Pointer with value pointer
 * 
 * Returns:
 *     Error, if any.
 */
error_t t_variable_get_ptr(const tok_t * toks, const tok_t ** next_toks, numeric_t ** value);

#endif

