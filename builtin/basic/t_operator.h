#ifndef _T_OPERATOR_H
#define _T_OPERATOR_H

#include <stdint.h>

#include "t_defs.h"

#ifdef Z80
#define TGT_FASTCALL __z88dk_fastcall
#else
#define TGT_FASTCALL
#endif

typedef uint8_t operator_t;

#define OP_MINUS 0x00
#define OP_PLUS 0x01
#define OP_EQUAL 0x02
#define OP_LPAREN 0x03
#define OP_RPAREN 0x04
#define OP_LT 0x05
#define OP_GT 0x06
#define OP_LTEQ 0x07
#define OP_GTEQ 0x08
#define OP_MULT 0x09

#define OP_CHECK(_toks, _optype) ((*_toks == TOK_OPERATOR) && (*(_toks+1) == _optype))
#define OP_GET(_toks) (*(_toks+1))
#define OP_SIZE 2

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

/* t_operator_is_comparison
 *
 * Purpose:
 *     Check if an operator token is a comparison
 *     operator.
 * 
 * Parameters:
 *     toks:   Pointer to operator token.
 * 
 * Returns:
 *     true if comparison, false otherwise.
 */
uint8_t t_operator_is_comparison(const tok_t * toks) TGT_FASTCALL;

#endif

