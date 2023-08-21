#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "program.h"
#include "eval.h"
#include "statement.h"

#include "t_defs.h"
#include "t_keyword.h"
#include "t_numeric.h"
#include "t_operator.h"

const Keyword_T keywords[NUM_KEYWORDS] =
{
    {
        "PRINT",
        KEYWORD_PRINT
    },
    {
        "LIST",
        KEYWORD_LIST
    },
    {
        "NEW",
        KEYWORD_NEW
    },
    {
        "RUN",
        KEYWORD_RUN
    },
    {
        "END",
        KEYWORD_END
    },
    {
        "GOTO",
        KEYWORD_GOTO
    },
    {
        "FOR",
        KEYWORD_FOR
    },
    {
        "TO",
        KEYWORD_TO
    },
    {
        "NEXT",
        KEYWORD_NEXT
    },
    {
        "GOSUB",
        KEYWORD_GOSUB
    },
    {
        "RETURN",
        KEYWORD_RETURN
    },
    {
        "DIM",
        KEYWORD_DIM
    },
    {
        "IF",
        KEYWORD_IF
    },
    {
        "THEN",
        KEYWORD_THEN
    }
};

int t_keyword_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    char kw_str[11];
    size_t len = 0;

    const char * input = *input_ptr;
    tok_t * dst = *dst_ptr;

    while (*input >= 'A' && *input <= 'Z')
    {
        kw_str[len] = *input;
        input++;
        len++;
    }

    kw_str[len] = '\0';

    kw_code code = KEYWORD_UNDEFINED;

    for (int i = 0; i < NUM_KEYWORDS; i++)
    {
        if (strcmp(kw_str, keywords[i].str) == 0)
        {
            code = keywords[i].code;
            break;
        }
    }

    if (code != KEYWORD_UNDEFINED)
    {
        *dst = TOK_KEYWORD;
        dst++;
        *dst = code;
        dst++;

        *dst_ptr = dst;
        *input_ptr = input;

        return 1;
    }

    return 0;
}

error_t do_print_string(const tok_t * toks)
{
    /* Size of the string. */
    tok_size_t size = *toks;
    toks++;

    /* Now print the string. */
    for (tok_size_t i = 0; i < size; i++)
    {
        char c = *toks;
        toks++;
        putchar(c);
    }

    return ERROR_NOERROR;
}

error_t do_print_numeric(const tok_t * toks)
{
    /* Evaluate the statement */
    numeric_t result;
    error_t e = eval_numeric(&result, toks);

    if (e != ERROR_NOERROR) return e;

    /* Print the result. */
    printf("%d", result);

    return ERROR_NOERROR;
}

error_t do_print(const tok_t * toks)
{
    while (1)
    {
        /* First token in pair must be a string
         * or a variable.
         */
        if (*toks == TOK_STRING)
        {
            error_t e = do_print_string(toks+1);
            if (e != ERROR_NOERROR) return e;
        }
        else
        {
            error_t e = do_print_numeric(toks);
            if (e != ERROR_NOERROR) return e;
        }

        /* Skip ahead to either a separator or a terminator. */
        while (*toks != TOK_TERMINATOR && *toks != TOK_SEPARATOR)
        {
            toks += t_defs_size(toks);
        }

        /* Stop print loop if we've hit a terminator. */
        if (*toks == TOK_TERMINATOR) break;

        /* Skip separator so that we evaluate the next expression */
        toks += 1;
    }

    putchar('\r');
    putchar('\n');

    return ERROR_NOERROR;
}

error_t do_list(const tok_t * toks)
{
    program_list();
    return ERROR_NOERROR;
}

error_t do_new(const tok_t * toks)
{
    program_new();
    return ERROR_NOERROR;
}

error_t do_run(const tok_t * toks)
{
    return program_run();
}

error_t do_end(const tok_t * toks)
{
    return program_end(ERROR_NOERROR);
}

error_t do_goto(const tok_t * toks)
{
    /* Next token must be numeric. */
    tok_t tok_type = *toks;

    if (tok_type != TOK_NUMERIC) return ERROR_SYNTAX;

    int lineno = NUMERIC_GET(toks);
    program_set_next_lineno(lineno);
    return ERROR_NOERROR;
}

error_t do_for(const tok_t * toks)
{
    /* Get some info about the for loop. */
    /* Syntax is FOR <VAR> = <NUM> TO <NUM> */
    
    /* Get variable name. */
    char varname[VARNAME_BUF_SIZE];
    if (*toks != TOK_VARIABLE) return ERROR_SYNTAX;
    t_variable_get(varname, toks+1);
    toks += t_defs_size(toks);

    /* Next should be EQUALS. */
    if (*toks != TOK_OPERATOR) return ERROR_SYNTAX;
    if (*(toks+1) != OP_EQUAL) return ERROR_SYNTAX;
    toks += t_defs_size(toks);

    /* Now NUMERIC. */
    if (*toks != TOK_NUMERIC) return ERROR_SYNTAX;
    numeric_t start = NUMERIC_GET(toks);
    toks += t_defs_size(toks);

    /* Now TO. */
    if (*toks != TOK_KEYWORD) return ERROR_SYNTAX;
    if (*(toks+1) != KEYWORD_TO) return ERROR_SYNTAX;
    toks += t_defs_size(toks);

    /* Finally limit NUMERIC. */
    if (*toks != TOK_NUMERIC) return ERROR_SYNTAX;
    numeric_t limit = NUMERIC_GET(toks);
    toks += t_defs_size(toks);

    /* Get top of return stack to see if we're
     * already in the loop or not. */
    program_return_t tos;
    error_t e_onstack = program_pop_return(&tos);

    /* If the variable in the FOR matches
     * the variable in TOS, then we're already
     * in the loop. */
    if (e_onstack == ERROR_NOERROR && strcmp(tos.varname, varname) == 0)
    {
        numeric_t current_val;
        program_get_numeric(varname, &current_val);
        program_set_numeric(varname, current_val + 1);
    }
    else
    {
        program_set_numeric(varname, start);

        /* Push TOS back onto stack (if there was one) -
         * it's nothing to do with this loop. */
        if (e_onstack == ERROR_NOERROR) program_push_return(&tos);
    }

    /* Now check if we've hit the limit. */
    numeric_t current_val;
    program_get_numeric(varname, &current_val);
    if (current_val >= limit)
    {
        program_set_next_lineno(tos.lineno+1);
    }
    else
    {
        program_return_t new_tos;
        new_tos.lineno = program_current_lineno();
        strcpy(new_tos.varname, varname);
        program_push_return(&new_tos);
    }

    return ERROR_NOERROR;
}

/* Handler to use when a keyword is valid but cannot be interpreted
 * by itself (e.g. TO, THEN)
 */
error_t cannot_interpret(const tok_t * toks)
{
    return ERROR_SYNTAX;
}

error_t do_next(const tok_t * toks)
{
    /* Should be followed by a variable. */
    if (*toks != TOK_VARIABLE) return ERROR_SYNTAX;

    /* Get variable name. */
    char varname[VARNAME_BUF_SIZE];
    t_variable_get(varname, toks+1);

    /* Pop return value off stack.
     * This should have been set by a FOR. */
    program_return_t ret;
    error_t e = program_pop_return(&ret);

    /* If we get an empty-stack error then we have */
    /* a NEXT without a FOR. */
    if (e == ERROR_RETSTACK_EMPTY) return ERROR_SYNTAX;
    ERROR_HANDLE(e);

    /* Check it's the right variable! */
    if (strcmp(varname, ret.varname) != 0) return ERROR_SYNTAX;

    numeric_t dest = ret.lineno;

    /* Push a new value onto the stack. */
    ret.lineno = program_current_lineno();
    e = program_push_return(&ret);
    ERROR_HANDLE(e);

    /* GOTO the loop start. */
    program_set_next_lineno(dest);

    return ERROR_NOERROR;
}

error_t do_gosub(const tok_t * toks)
{
    /* Check that next token is a line number. */
    if (*toks != TOK_NUMERIC) return ERROR_SYNTAX;

    /* Get line number from token stream. */
    numeric_t next_lineno = NUMERIC_GET(toks);
    numeric_t current_lineno = program_current_lineno();
    
    /* Push current line number onto stack. */
    program_return_t ret;
    memset(ret.varname, 0, VARNAME_BUF_SIZE);
    ret.lineno = current_lineno;

    /* Push return location onto stack. */
    program_push_return(&ret);

    /* Transfer control to destination line. */
    program_set_next_lineno(next_lineno);
    
    return ERROR_NOERROR;
}

error_t do_return(const tok_t * toks)
{
    /* Pop top value off stack. */
    program_return_t ret;
    error_t e = program_pop_return(&ret);
    if (e != ERROR_NOERROR) return e;

    /* Transfer control to return line. */
    program_set_next_lineno(ret.lineno+1);
    
    return ERROR_NOERROR;
}

error_t do_dim(const tok_t * toks)
{
    /* Variable name */
    char varname[VARNAME_BUF_SIZE];
    t_variable_get(varname, toks+1);
    toks += t_defs_size(toks);

    /* Operator ( */
    toks += t_defs_size(toks);

    /* Size (numeric). */
    numeric_t size = NUMERIC_GET(toks);
    toks += t_defs_size(toks);

    /* Operator ) */
    toks += t_defs_size(toks);

    /* Check size is in range.
     * We can allocate a maximum of 255 bytes.
     */
    if (size > (255 / sizeof(numeric_t)))
    {
        return ERROR_RANGE;
    }

    /* Create a new array of the right size. */
    /* We multiply by the size of a numeric because it's an array
     * of numerics. */
    program_create_array(varname, size * sizeof(numeric_t));

    return ERROR_NOERROR;
}

static const tok_t * helper_if_get_operator(const tok_t * toks)
{
    while (!(OP_CHECK(toks, OP_EQUAL)
             || OP_CHECK(toks, OP_LT)
             || OP_CHECK(toks, OP_GT)
             || OP_CHECK(toks, OP_LTEQ)
             || OP_CHECK(toks, OP_GTEQ)))
    {
        /* If we hit a terminator then there is no operator. */
        if (*toks == TOK_TERMINATOR) return TOK_PTR_NULL;

        /* If we hit THEN then there is no operator (at least not in the if-expression) */
        if (KW_CHECK(toks, KEYWORD_THEN)) return TOK_PTR_NULL;

        SKIP(toks);
    }
    return toks;
}

static const tok_t * helper_if_get_then(const tok_t * toks)
{
    while (!KW_CHECK(toks, KEYWORD_THEN))
    {
        /* If we hit a terminator then there is no THEN. */
        if (*toks == TOK_TERMINATOR) return TOK_PTR_NULL;

        SKIP(toks);
    }
    return toks;
}

/* Get a sequence of tokens from start to end (end not inclusive). */
static void helper_if_get_toks(tok_t * dst, const tok_t * start, const tok_t * end)
{
    size_t size = (size_t)(end - start);
    memcpy(dst, start, size);
    dst[size] = TOK_TERMINATOR;
}

/* Performs the given comparison on two values. */
static int helper_if_do_comparison(const tok_t * comp, numeric_t left, numeric_t right)
{
    operator_t op = *(comp+1);
    switch(op)
    {
        case OP_EQUAL:
            return left == right;
        case OP_LT:
            return left < right;
        case OP_GT:
            return left > right;
        case OP_LTEQ:
            return left <= right;
        case OP_GTEQ:
            return left >= right;
        default:
            return -1;
    }
}

error_t do_if(const tok_t * toks)
{
    /* Syntax is:
     * (IF) expr = expr THEN stmt
     */

    /* Get position of comparison operator. */
    const tok_t * pos_comp = helper_if_get_operator(toks);
    if (pos_comp == TOK_PTR_NULL) return ERROR_SYNTAX;

    /* Get position of THEN. */
    const tok_t * pos_then = helper_if_get_then(toks);
    if (pos_then == TOK_PTR_NULL) return ERROR_SYNTAX;

    /* Get both expressions. */
    tok_t expr1[100];
    tok_t expr2[100];
    helper_if_get_toks(expr1, toks, pos_comp);
    helper_if_get_toks(expr2, pos_comp+2, pos_then);
    
    /* Evaluate both expressions. */
    numeric_t expr1_val;
    numeric_t expr2_val;
    ERROR_HANDLE(eval_numeric(&expr1_val, expr1));
    ERROR_HANDLE(eval_numeric(&expr2_val, expr2));
    
    /* Do the comparison. */
    int result = helper_if_do_comparison(pos_comp, expr1_val, expr2_val);

    /* Non-zero result means that the comparison was invalid
     * (somehow - we checked it earlier!)
     */
    if (result < 0) return ERROR_SYNTAX;

    /* If true, interpret the sub-statement. */
    if (result)
    {
        /* Get tokens for sub-statement. */
        const tok_t * sub_stmt = pos_then + 2;
        ERROR_HANDLE(statement_interpret(sub_stmt));
    }

    return ERROR_NOERROR;
}

const f_interpreter_t keyword_funcs[NUM_KEYWORDS] =
{
    do_print,
    do_list,
    do_new,
    do_run,
    do_end,
    do_goto,
    do_for,
    cannot_interpret, /* TO */
    do_next,
    do_gosub,
    do_return,
    do_dim,
    do_if,
    cannot_interpret /* THEN */
};

error_t t_keyword_interpret(kw_code kw, const tok_t * toks)
{
    uint8_t index = kw - KEYWORD_BASE;
    if (index >= NUM_KEYWORDS) return ERROR_UNDEFINED_KW;

    return keyword_funcs[index](toks);
}

const tok_t * t_keyword_list(const tok_t * toks)
{
    kw_code kw = *toks;
    toks++;

    for (int i = 0; i < NUM_KEYWORDS; i++)
    {
        if (kw == keywords[i].code)
        {
            printf("%s", keywords[i].str);
            break;
        }
    }

    putchar(' ');

    return toks;
}
