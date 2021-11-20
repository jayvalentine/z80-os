#ifndef _T_OPERATOR_H
#define _T_OPERATOR_H

#include <stdint.h>

#include "t_defs.h"

typedef uint8_t operator_t;

#define OP_MINUS 0x00
#define OP_PLUS 0x01
#define OP_EQUAL 0x02

/* t_operator_parse
 *
 * Purpose:
 *     Parse an operator from the given input stream.
 *     Operator token is placed in the output stream.
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
int t_operator_parse(tok_t ** dst_ptr, const char ** input_ptr);

/* t_operator_size
 *
 * Purpose:
 *     Get the size of an operator token.
 * 
 * Parameters:
 *     toks:   Operator token stream.
 * 
 * Returns:
 *     Size of operator token.
 */
tok_size_t t_operator_size(const tok_t * toks);

/* t_operator_list
 *
 * Purpose:
 *     Print an operator token.
 * 
 * Parameters:
 *     toks:   Operator token stream.
 * 
 * Returns:
 *     Pointer to token after the operator.
 */
const tok_t * t_operator_list(const tok_t * toks);

#endif

