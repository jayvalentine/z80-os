#include "t_func.h"

#include "eval.h"

#include "t_operator.h"

#include <string.h>
#include <stdio.h>

int t_func_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    const char * input = *input_ptr;

    /* Get next whitespace, null, or non-A-Z char. */
    const char * p = *input_ptr;
    while (*p != ' ' && *p != '\0' && *p >= 'A' && *p <= 'Z') p++;

    /* Copy function name */
    char funcname[FUNCNAME_BUF_SIZE];
    size_t size = (size_t)(p - input);
    memcpy(funcname, input, size);
    funcname[size] = '\0';

    /* Check function name. */
    if (strcmp(funcname, "RND") == 0)
    {
        tok_t * dst = *dst_ptr;
        *dst = TOK_FUNC; dst++;
        *dst = FUNC_RND; dst++;

        *dst_ptr = dst;
        *input_ptr = p;
        
        return 1;
    }

    /* Not a function. */
    return 0;
}

#define RND_RANGE_MIN 1

uint16_t r;

void srand16(uint16_t seed)
{
    r = seed;
}

uint16_t rand16(void)
{
    r ^= r << 7;
    r ^= r >> 9;
    r ^= r << 8;
    return r;
}

error_t t_func_call(const tok_t * toks, const tok_t ** end, numeric_t * ret)
{
    if (*toks != TOK_FUNC) return ERROR_SYNTAX;
    toks++;

    if (*toks == FUNC_RND)
    {
        toks++;
        
        /* Is the next token a left paren?
         * If so then evaluate the argument.
         */
        if (OP_CHECK(toks, OP_LPAREN))
        {
            numeric_t param;
            const tok_t * expr_end;
            ERROR_HANDLE(eval_numeric(&param, toks, &expr_end));

            *ret = (numeric_t)(rand16() % (param + 1 - RND_RANGE_MIN) + RND_RANGE_MIN);

            /* Skip to next rparen. */
            toks = expr_end;

            /* Skip the rparen, set pointer to token after call. */
            toks += OP_SIZE;
            *end = toks;

            return ERROR_NOERROR;
        }
        else
        {
            uint16_t v = rand16();
            *ret = *(numeric_t *)&v;

            /* Set pointer to token after call (current token). */
            *end = toks;
            
            return ERROR_NOERROR;
        }
    }

    return ERROR_UNKNOWN_FUNC;
}

/* Print function token. */
const tok_t * t_func_list(const tok_t * toks)
{
    const char * s;
    if (*toks == FUNC_RND) s = "RND";
    else s = "FUNC?";

    printf("%s", s);

    toks++;
    return toks;
}
