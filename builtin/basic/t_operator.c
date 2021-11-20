#include "t_operator.h"

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
    char * dst = *dst_ptr;

    char c = *input;

    if (c == '-')
    {
        op = OP_MINUS;
        input++;
    }
    else
    {
        return 0;
    }

    *dst = TOK_OPERATOR;
    dst++;
    *dst = op;
    dst++;

    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
}
