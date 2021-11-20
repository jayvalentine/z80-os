#include "eval.h"

#include "t_numeric.h"
#include "t_operator.h"

#define OPSTACK_MAX 16
#define NUMSTACK_MAX 16

typedef struct _OPSTACK_T
{
    uint8_t count;
    operator_t stack[OPSTACK_MAX];
} opstack_t;

typedef struct _NUMSTACK_T
{
    uint8_t count;
    numeric_t stack[NUMSTACK_MAX];
} numstack_t;

uint8_t opstack_push(opstack_t * opstack, operator_t op)
{
    if (opstack->count == OPSTACK_MAX) return 0;
    opstack->stack[opstack->count] = op;
    opstack->count++;
    return 1;
}

uint8_t opstack_pop(opstack_t * opstack, operator_t * op)
{
    if (opstack->count == 0) return 0;
    opstack->count--;
    *op = opstack->stack[opstack->count];
    return 1;
}

uint8_t numstack_push(numstack_t * numstack, numeric_t num)
{
    if (numstack->count == NUMSTACK_MAX) return 0;
    numstack->stack[numstack->count] = num;
    numstack->count++;
    return 1;
}

uint8_t numstack_pop(numstack_t * numstack, numeric_t * num)
{
    if (numstack->count == 0) return 0;
    numstack->count--;
    *num = numstack->stack[numstack->count];
    return 1;
}

/* eval_numeric
 *
 * Purpose:
 *     Attempts to evaluate the expression given
 *     by the tokens at src and write the result
 *     as a numeric token to dst.
 * 
 * Parameters:
 *     dst: Destination for result token.
 *     src: Expression tokens to evaluate.
 * 
 * Returns:
 *     Error, if any.
 */
error_t eval_numeric(tok_t * dst, const tok_t * src)
{
    opstack_t opstack;
    opstack.count = 0;

    tok_t output[256];
    tok_t * output_ptr = &output[0];

    while (1)
    {
        tok_t tok = *src;

        if (tok == TOK_TERMINATOR) break;

        if (tok == TOK_NUMERIC)
        {
            numeric_t num = t_numeric_get(src+1);
            t_numeric_put(output_ptr, num);
            output_ptr += t_numeric_size(output_ptr) + 1;
        }
        else if (tok == TOK_OPERATOR)
        {
            operator_t op = *(src+1);
            opstack_push(&opstack, op);
        }

        src += t_defs_size(src);
    }

    /* At this point we have a numeric
     * in the output stream. */

    *output_ptr = TOK_OPERATOR;
    output_ptr++;
    opstack_pop(&opstack, output_ptr);
    output_ptr++;

    *output_ptr = TOK_TERMINATOR;

    /* Evaluate the output, result is TOS. */
    output_ptr = &output[0];
    numstack_t numstack;
    numstack.count = 0;

    while (1)
    {
        tok_t tok = *output_ptr;

        if (tok == TOK_TERMINATOR) break;

        if (tok == TOK_NUMERIC)
        {
            numeric_t num = t_numeric_get(output_ptr+1);
            numstack_push(&numstack, num);
        }
        else if (tok == TOK_OPERATOR)
        {
        }

        output_ptr += t_defs_size(output_ptr);
    }

    /* Get TOS. */
    numeric_t result;
    numstack_pop(&numstack, &result);

    t_numeric_put(dst, result);

    return ERROR_NOERROR;
}