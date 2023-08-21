#include <string.h>

#include <stdio.h>

#include "eval.h"
#include "program.h"

#include "t_numeric.h"
#include "t_operator.h"
#include "t_variable.h"
#include "t_func.h"

#include "array.h"

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

/* Gets top of given opstack. */
#define OPSTACK_TOP(_opstack) (_opstack.stack[_opstack.count-1])

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

#define NUMSTACK_PUSH(_numstack, _num) \
    _numstack.stack[_numstack.count++] = _num

#define NUMSTACK_POP(_numstack) \
    _numstack.stack[--(_numstack.count)]

static int8_t helper_eval_precedence(operator_t op)
{
    switch (op)
    {
        case OP_MULT:
            return 4;
        case OP_MINUS:
            return 2;
        case OP_PLUS:
            return 1;
        default:
            return 0;
    }
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
    tok_t tok;
    numeric_t num;

    opstack_t opstack;
    opstack.count = 0;

    tok_t output[256];
    tok_t * output_ptr = &output[0];

    while (1)
    {
        tok = *src;

        if (tok == TOK_TERMINATOR) break;
        if (tok == TOK_SEPARATOR) break;
        if (tok == TOK_OPERATOR && *(src+1) == OP_RPAREN) break;

        if (tok == TOK_NUMERIC)
        {
            num = NUMERIC_GET(src);
            t_numeric_put(output_ptr, num);
            output_ptr += 3;

            /* Skip numeric token */
            SKIP(src);
        }
        else if (tok == TOK_FUNC)
        {
            numeric_t val;
            const tok_t * end;
            ERROR_HANDLE(t_func_call(src, &end, &val));

            t_numeric_put(output_ptr, val);
            output_ptr += 3;

            /* Skip function call. */
            src = end;
        }
        else if (tok == TOK_VARIABLE)
        {
            numeric_t * val;
            const tok_t * next_toks;
            ERROR_HANDLE(t_variable_get_ptr(src, &next_toks, &val));

            t_numeric_put(output_ptr, *val);
            output_ptr += 3;

            /* Skip var ref tokens */
            src = next_toks;
        }
        else if (tok == TOK_OPERATOR)
        {
            /* Pop operators off the stack (into output) while they have greater precedence
             * than this operator. */
            while (opstack.count > 0
                   && (helper_eval_precedence(OPSTACK_TOP(opstack)) > helper_eval_precedence(OP_GET(src))))
            {
                operator_t op;
                uint8_t res = opstack_pop(&opstack, &op);
                if (!res) return ERROR_SYNTAX;

                *output_ptr = TOK_OPERATOR;
                output_ptr++;
                *output_ptr = op;
                output_ptr++;
            }
            
            /* Push this operator. */
            operator_t op = OP_GET(src);
            opstack_push(&opstack, op);

            /* Skip operator token */
            SKIP(src);
        }
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
        tok = *output_ptr;

        if (tok == TOK_TERMINATOR) break;

        if (tok == TOK_NUMERIC)
        {
            num = NUMERIC_GET(output_ptr);
            NUMSTACK_PUSH(numstack, num);
        }
        else if (tok == TOK_OPERATOR)
        {
            operator_t op = *(output_ptr+1);

            if (op == OP_PLUS)
            {
                /* Pop 2 numbers off stack, add them together. */
                numeric_t a = NUMSTACK_POP(numstack);
                numeric_t b = NUMSTACK_POP(numstack);

                numeric_t c = a + b;
                NUMSTACK_PUSH(numstack, c);
            }

            else if (op == OP_MULT)
            {
                /* Pop 2 numbers off stack, multiply. */
                numeric_t a = NUMSTACK_POP(numstack);
                numeric_t b = NUMSTACK_POP(numstack);

                numeric_t c = a * b;
                NUMSTACK_PUSH(numstack, c);
            }

            else if (op == OP_MINUS)
            {
                /* How many values on stack? */
                if (numstack.count == 1)
                {
                    /* Just one, so this is negation. */
                    numeric_t a = NUMSTACK_POP(numstack);

                    a = -a;

                    NUMSTACK_PUSH(numstack, a);
                }
                else if (numstack.count >= 2)
                {
                    /* Pop 2 numbers off stack, add them together. */
                    numeric_t a = NUMSTACK_POP(numstack);
                    numeric_t b = NUMSTACK_POP(numstack);

                    numeric_t c = b - a;
                    NUMSTACK_PUSH(numstack, c);
                }
            }
        }

        SKIP(output_ptr);
    }

    /* Get TOS. */
    *result = NUMSTACK_POP(numstack);

    return ERROR_NOERROR;
}