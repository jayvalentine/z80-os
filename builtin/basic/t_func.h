#ifndef _T_FUNC_H
#define _T_FUNC_H

#include "errors.h"

#include "t_defs.h"
#include "t_numeric.h"

#define FUNCNAME_SIZE 8
#define FUNCNAME_BUF_SIZE (FUNCNAME_SIZE+1)

/* Builtin functions. */
#define FUNC_RND 0x01

/* t_func_parse
 *
 * Purpose:
 *     Parse a function from the given input stream.
 *     Function token is placed in the output stream.
 *     Both stream pointers are updated to point to the
 *     token/character after the processed ones.
 * 
 * Parameters:
 *     dst_ptr:   Reference to pointer in destination stream.
 *     input_ptr: Reference to pointer in input stream.
 * 
 * Returns:
 *     1 if successful, 0 otherwise.
 */
int t_func_parse(tok_t ** dst_ptr, const char ** input_ptr);

/* t_func_call
 *
 * Purpose:
 *     Call a function.
 * 
 * Parameters:
 *     toks: Function token representing callee function.
 *     end:  Pointer to populate with pointer to token after function call.
 *     ret:  Pointer to populate with return value.
 * 
 * Returns:
 *     Error, if any.
 */
error_t t_func_call(const tok_t * toks, const tok_t ** end, numeric_t * ret);

/* Print function token. */
const tok_t * t_func_list(const tok_t * toks);

/* Function token size. */
tok_t t_func_size(const tok_t * toks);

#endif /* _T_FUNC_H */
