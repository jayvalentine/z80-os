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

/* Macros for accessing stacks.
 * Stacks are accessed via the pointer for better performance
 * on targets where variable array indexing is expensive.
 * 
 * Stack pointer always points to the top value on the stack.
 */

#define PTR(_stack) _stack ## _ptr

#define STACK_INIT(_stack) \
    PTR(_stack) = (&(_stack.stack[0]) - 1); \
    _stack.count = 0

/* Gets top of given opstack. */
#define STACK_TOP(_stack) (*PTR(_stack))

#define STACK_PUSH_PTR(_stack, _stackptr, _val) \
    *(++_stackptr) = _val; \
    _stack.count++

#define STACK_PUSH(_stack, _val) \
    STACK_PUSH_PTR(_stack, PTR(_stack), _val)

#define STACK_POP_PTR(_stack, _stackptr) \
    *(_stackptr--); \
    _stack.count--

#define STACK_POP(_stack) \
    STACK_POP_PTR(_stack, PTR(_stack))

inline static int8_t helper_eval_precedence(operator_t op)
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

/* Helper method that returns true if the given token
 * marks the end of an expression.
 */
inline static uint8_t eval_is_end_of_expression(const tok_t * toks)
{
    tok_t tok = *toks;

    if (tok == TOK_TERMINATOR) return 1;
    else if (tok == TOK_SEPARATOR) return 1;
    else if (tok == TOK_KEYWORD) return 1;
    else if (OP_CHECK(toks, OP_RPAREN)) return 1;
    else if (t_operator_is_comparison(toks)) return 1;

    return 0;
}

/* eval_shunting_yard
 *
 * Purpose:
 *     Helper method to implement the shunting yard algorithm for
 *     eval_numeric.
 * 
 * Parameters:
 *     src:    Expression tokens to evaluate
 *     output: Output pointer for resulting RPN expression
 *     end:    Pointer to token after evaluated expression
 * 
 * Returns:
 *     Error, if any.
 * 
 */
static error_t eval_shunting_yard(const tok_t * src, tok_t * output, const tok_t ** end)
{
    static const tok_t * working_src;
    static operator_t * working_opstack_ptr;

    opstack_t opstack;
    operator_t * opstack_ptr;
    STACK_INIT(opstack);

    working_src = src;
    working_opstack_ptr = opstack_ptr;

    while (1)
    {
        if (eval_is_end_of_expression(working_src)) break;

        tok_t tok = *working_src;

        if (tok == TOK_NUMERIC)
        {
            numeric_t num = NUMERIC_GET(working_src);
            *output = TOK_NUMERIC;
            output++;
            *(numeric_t *)(output) = num;
            output += (NUMERIC_SIZE - 1);

            /* Skip numeric token */
            working_src += NUMERIC_SIZE;
        }
        else if (tok == TOK_FUNC)
        {
            numeric_t val;
            const tok_t * end;

            /* Can eval, save opstack. */
            src = working_src;
            opstack_ptr = working_opstack_ptr;
            ERROR_HANDLE(t_func_call(src, &end, &val));
            working_opstack_ptr = opstack_ptr;
            working_src = src;

            *output = TOK_NUMERIC;
            *(numeric_t *)(output + 1) = val;
            output += NUMERIC_SIZE;

            /* Skip function call. */
            working_src = end;
        }
        else if (tok == TOK_VARIABLE || tok == TOK_REGISTER)
        {
            numeric_t * val;
            const tok_t * next_toks;

            /* Can eval, save opstack. */
            src = working_src;
            opstack_ptr = working_opstack_ptr;
            ERROR_HANDLE(t_variable_get_ptr(src, &next_toks, &val));
            working_opstack_ptr = opstack_ptr;
            working_src = src;

            *output = TOK_NUMERIC;
            *(numeric_t *)(output + 1) = *val;
            output += NUMERIC_SIZE;

            /* Skip var ref tokens */
            working_src = next_toks;
        }
        else if (tok == TOK_OPERATOR)
        {
            /* Pop operators off the stack (into output) while they have greater precedence
             * than this operator. */
            while (opstack.count > 0
                   && ((helper_eval_precedence(*working_opstack_ptr)) > helper_eval_precedence(OP_GET(working_src))))
            {
                operator_t op_old = STACK_POP_PTR(opstack, working_opstack_ptr);

                *output = TOK_OPERATOR;
                output++;
                *output = op_old;
                output++;
            }
            
            /* Push this operator. */
            operator_t op_new = OP_GET(working_src);
            STACK_PUSH_PTR(opstack, working_opstack_ptr, op_new);

            /* Skip operator token */
            working_src += OP_SIZE;
        }
    }

    /* Now pop all operators off the stack. */
    while (opstack.count > 0)
    {
        *output = TOK_OPERATOR;
        output++;
        operator_t op = STACK_POP_PTR(opstack, working_opstack_ptr);
        *output = op;
        output++;
    }

    *output = TOK_TERMINATOR;
    *end = working_src;

    return ERROR_NOERROR;
}

/* eval_rpn
 *
 * Purpose:
 *     Evaluates the given sequence of tokens representing
 *     a Reverse Polish Notation expression.
 * 
 * Parameters:
 *     expr:   Pointer to expression tokens to evaluate
 *     result: Destination for result value.
 * 
 * Returns:
 *     Error, if any.
 */
static error_t eval_rpn(const tok_t * expr, numeric_t * result)
{
    static numstack_t numstack;
    numeric_t * numstack_ptr;
    STACK_INIT(numstack);

    while (1)
    {
        tok_t tok = *expr;

        if (tok == TOK_TERMINATOR) break;

        if (tok == TOK_NUMERIC)
        {
            numeric_t num = NUMERIC_GET(expr);
            STACK_PUSH(numstack, num);

            expr += NUMERIC_SIZE;
        }
        else if (tok == TOK_OPERATOR)
        {
            operator_t op = *(expr+1);

            if (op == OP_PLUS)
            {
                /* Pop 2 numbers off stack, add them together. */
                numeric_t a = STACK_POP(numstack);
                numeric_t b = STACK_POP(numstack);

                numeric_t c = a + b;
                STACK_PUSH(numstack, c);
            }

            else if (op == OP_MULT)
            {
                /* Pop 2 numbers off stack, multiply. */
                numeric_t a = STACK_POP(numstack);
                numeric_t b = STACK_POP(numstack);

                numeric_t c = a * b;
                STACK_PUSH(numstack, c);
            }

            else if (op == OP_MINUS)
            {
                /* How many values on stack? */
                if (numstack.count == 1)
                {
                    /* Just one, so this is negation. */
                    numeric_t a = STACK_POP(numstack);

                    a = -a;

                    STACK_PUSH(numstack, a);
                }
                else if (numstack.count >= 2)
                {
                    /* Pop 2 numbers off stack, add them together. */
                    numeric_t a = STACK_POP(numstack);
                    numeric_t b = STACK_POP(numstack);

                    numeric_t c = b - a;
                    STACK_PUSH(numstack, c);
                }
            }

            expr += OP_SIZE;
        }
    }

    /* Get TOS. */
    *result = STACK_TOP(numstack);

    return ERROR_NOERROR;
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
    /* Shortcut evaluation if all we have is a single numeric or register variable. */
    if (*src == TOK_NUMERIC && eval_is_end_of_expression(src + NUMERIC_SIZE))
    {
        *result = NUMERIC_GET(src);
        *end = src + NUMERIC_SIZE;
        return ERROR_NOERROR;
    }
    else if (*src == TOK_REGISTER && eval_is_end_of_expression(src + REGISTER_SIZE))
    {
        numeric_t * val = program_get_numeric_ref(src);
        *result = *val;
        *end = src + REGISTER_SIZE;
        return ERROR_NOERROR;
    }

    tok_t output[256];

    const tok_t * tmp_end;
    ERROR_HANDLE(eval_shunting_yard(src, &output[0], &tmp_end));

    *end = tmp_end;

    /* Evaluate the output, get the result. */
    ERROR_HANDLE(eval_rpn(&output[0], result));

    return ERROR_NOERROR;
}