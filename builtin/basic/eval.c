#include <string.h>

#include "eval.h"
#include "program.h"

#include "t_numeric.h"
#include "t_operator.h"
#include "t_variable.h"

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

void opstack_pop_all(tok_t ** dst_ptr, opstack_t * opstack)
{
    tok_t * dst = *dst_ptr;
    while (opstack->count > 0)
    {
        *dst = TOK_OPERATOR;
        dst++;
        opstack_pop(opstack, dst);
        dst++;
    }

    *dst_ptr = dst;
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
 *     by the tokens at src and provides the result.
 * 
 * Parameters:
 *     result: Destination for result value
 *     src:    Expression tokens to evaluate.
 * 
 * Returns:
 *     Error, if any.
 */
error_t eval_numeric(numeric_t * result, const tok_t * src)
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
            output_ptr += 3;
        }
        else if (tok == TOK_VARIABLE)
        {
            /* Construct variable name string. */
            char varname[VARNAME_BUF_SIZE];
            t_variable_get(varname, src+1);

            /* Get variable value. */
            numeric_t val;
            error_t e = program_get_numeric(varname, &val);
            if (e != ERROR_NOERROR) return e;

            /* Write value to output. */
            t_numeric_put(output_ptr, val);
            output_ptr += 3;
        }
        else if (tok == TOK_OPERATOR)
        {
            /* Pop all operators off the stack. */
            opstack_pop_all(&output_ptr, &opstack);

            operator_t op = *(src+1);
            opstack_push(&opstack, op);
        }

        src += t_defs_size(src);
    }

    /* Now pop all operators off the stack. */
    opstack_pop_all(&output_ptr, &opstack);

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
            operator_t op = *(output_ptr+1);

            if (op == OP_PLUS)
            {
                /* Pop 2 numbers off stack, add them together. */
                numeric_t a;
                numeric_t b;

                numstack_pop(&numstack, &a);
                numstack_pop(&numstack, &b);

                numeric_t c = a + b;
                numstack_push(&numstack, c);
            }

            else if (op == OP_MINUS)
            {
                /* How many values on stack? */
                if (numstack.count == 1)
                {
                    /* Just one, so this is negation. */
                    numeric_t a;
                    numstack_pop(&numstack, &a);

                    a = -a;

                    numstack_push(&numstack, a);
                }
                else if (numstack.count == 2)
                {
                    /* Pop 2 numbers off stack, add them together. */
                    numeric_t a;
                    numeric_t b;

                    numstack_pop(&numstack, &a);
                    numstack_pop(&numstack, &b);

                    numeric_t c = b - a;
                    numstack_push(&numstack, c);
                }
            }
        }

        output_ptr += t_defs_size(output_ptr);
    }

    /* Get TOS. */
    numstack_pop(&numstack, result);

    return ERROR_NOERROR;
}