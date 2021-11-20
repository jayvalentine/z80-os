#ifndef _T_VARIABLE_H
#define _T_VARIABLE_H

#include <stdint.h>

#include "t_defs.h"

#define VARNAME_SIZE 4
#define VARNAME_BUF_SIZE (VARNAME_SIZE+1)

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

/* t_variable_size
 *
 * Purpose:
 *     Get the size of a variable token.
 * 
 * Parameters:
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Size of variable token.
 */
tok_size_t t_variable_size(const tok_t * toks);

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

#endif

