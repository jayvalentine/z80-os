#include <string.h>

#include <stdio.h>

#ifdef Z80
#define TGT_FASTCALL __z88dk_fastcall
#else
#define TGT_FASTCALL
#endif

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

#define OPSTACK_PUSH(_opstack, _op) \
    _opstack.stack[_opstack.count++] = _op

#define OPSTACK_POP(_opstack) \
    _opstack.stack[--(_opstack.count)]

#define NUMSTACK_PUSH(_numstack, _num) \
    _numstack.stack[_numstack.count++] = _num

#define NUMSTACK_POP(_numstack) \
    _numstack.stack[--(_numstack.count)]

int8_t helper_eval_precedence(operator_t op) TGT_FASTCALL
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
 *     end:    Token after end of expression.
 * 
 * Returns:
 *     Error, if any.
 */
error_t eval_numeric(numeric_t * result, const tok_t * src, const tok_t ** end)
{
    tok_t tok;
    numeric_t num;

    opstack_t opstack;
    opstack.count = 0;

    tok_t output[256];
    tok_t * output_ptr = &output[0];

    static opstack_t working_opstack;
    static tok_t * working_output_ptr;
    static const tok_t * working_src;

    memcpy(&working_opstack, &opstack, sizeof(opstack_t));
    working_output_ptr = output_ptr;
    working_src = src;

    while (1)
    {
        tok = *working_src;

        if (tok == TOK_TERMINATOR) break;
        if (tok == TOK_SEPARATOR) break;
        if (OP_CHECK(working_src, OP_RPAREN)) break;
        if (tok == TOK_KEYWORD) break;
        if (t_operator_is_comparison(working_src)) break;

        if (tok == TOK_NUMERIC)
        {
            num = NUMERIC_GET(working_src);
            *working_output_ptr = TOK_NUMERIC;
            working_output_ptr++;
            *(numeric_t *)(working_output_ptr) = num;
            working_output_ptr += (NUMERIC_SIZE - 1);

            /* Skip numeric token */
            working_src += NUMERIC_SIZE;
        }
        else if (tok == TOK_FUNC)
        {
            numeric_t val;
            const tok_t * end;

            /* Can eval, save opstack. */
            memcpy(&opstack, &working_opstack, sizeof(opstack_t));
            output_ptr = working_output_ptr;
            src = working_src;
            ERROR_HANDLE(t_func_call(src, &end, &val));
            working_src = src;
            working_output_ptr = output_ptr;
            memcpy(&working_opstack, &opstack, sizeof(opstack_t));

            *working_output_ptr = TOK_NUMERIC;
            *(numeric_t *)(working_output_ptr + 1) = val;
            working_output_ptr += NUMERIC_SIZE;

            /* Skip function call. */
            working_src = end;
        }
        else if (tok == TOK_VARIABLE || tok == TOK_REGISTER)
        {
            numeric_t * val;
            const tok_t * next_toks;

            /* Can eval, save opstack. */
            memcpy(&opstack, &working_opstack, sizeof(opstack_t));
            output_ptr = working_output_ptr;
            src = working_src;
            ERROR_HANDLE(t_variable_get_ptr(src, &next_toks, &val));
            working_src = src;
            working_output_ptr = output_ptr;
            memcpy(&working_opstack, &opstack, sizeof(opstack_t));

            *working_output_ptr = TOK_NUMERIC;
            *(numeric_t *)(working_output_ptr + 1) = *val;
            working_output_ptr += NUMERIC_SIZE;

            /* Skip var ref tokens */
            working_src = next_toks;
        }
        else if (tok == TOK_OPERATOR)
        {
            /* Pop operators off the stack (into output) while they have greater precedence
             * than this operator. */
            while (working_opstack.count > 0
                   && (helper_eval_precedence(OPSTACK_TOP(working_opstack)) > helper_eval_precedence(OP_GET(working_src))))
            {
                operator_t op = OPSTACK_POP(working_opstack);

                *working_output_ptr = TOK_OPERATOR;
                working_output_ptr++;
                *working_output_ptr = op;
                working_output_ptr++;
            }
            
            /* Push this operator. */
            operator_t op = OP_GET(working_src);
            OPSTACK_PUSH(working_opstack, op);

            /* Skip operator token */
            working_src += OP_SIZE;
        }
    }

    /* Now pop all operators off the stack. */
    while (working_opstack.count > 0)
    {
        *working_output_ptr = TOK_OPERATOR;
        working_output_ptr++;
        operator_t op = working_opstack.stack[--(working_opstack.count)];
        *working_output_ptr = op;
        working_output_ptr++;
    }

    *working_output_ptr = TOK_TERMINATOR;

    /* Evaluate the output, result is TOS. */
    working_output_ptr = &output[0];
    numstack_t numstack;
    numstack.count = 0;

    while (1)
    {
        tok = *working_output_ptr;

        if (tok == TOK_TERMINATOR) break;

        if (tok == TOK_NUMERIC)
        {
            num = NUMERIC_GET(working_output_ptr);
            NUMSTACK_PUSH(numstack, num);

            working_output_ptr += NUMERIC_SIZE;
        }
        else if (tok == TOK_OPERATOR)
        {
            operator_t op = *(working_output_ptr+1);

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

            working_output_ptr += OP_SIZE;
        }
    }

    /* Get TOS. */
    *result = NUMSTACK_POP(numstack);

    *end = working_src;
    return ERROR_NOERROR;
}