#include "t_operator.h"

#include <stdio.h>

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
int t_operator_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    operator_t op;

    const char * input = *input_ptr;
    tok_t * dst = *dst_ptr;

    char c = *input;

    switch (c)
    {
        case '-': op = OP_MINUS; break;
        case '+': op = OP_PLUS; break;
        case '=': op = OP_EQUAL; break;
        case '(': op = OP_LPAREN; break;
        case ')': op = OP_RPAREN; break;
        default: return 0;
    }

    input++;

    *dst = TOK_OPERATOR;
    dst++;
    *dst = op;
    dst++;

    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
}

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
tok_size_t t_operator_size(const tok_t * toks)
{
    return 1;
}


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
const tok_t * t_operator_list(const tok_t * toks)
{
    operator_t op = *toks;
    toks++;

    if (op == OP_MINUS)
    {
        putchar('-');
    }
    else if (op == OP_PLUS)
    {
        putchar('+');
    }
    else if (op == OP_EQUAL)
    {
        putchar('=');
    }

    putchar(' ');

    return toks;
}
