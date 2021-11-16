#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "program.h"

#include "t_defs.h"
#include "t_keyword.h"
#include "t_numeric.h"

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
    }
};

int t_keyword_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    char kw_str[11];
    size_t len = 0;

    const char * input = *input_ptr;
    tok_t * dst = *dst_ptr;

    while (*input != ' ' && *input != '\0')
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

error_t do_print(const tok_t * toks)
{
    /* Next token must be a string. */
    if (*toks != TOK_STRING) return ERROR_SYNTAX;
    toks++;

    /* Now size of the string. */
    tok_size_t size = *toks;
    toks++;

    /* Now print the string. */
    for (tok_size_t i = 0; i < size; i++)
    {
        char c = *toks;
        toks++;
        putchar(c);
    }

    puts("\r\n");

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
    toks++;

    if (tok_type != TOK_NUMERIC) return ERROR_SYNTAX;

    int lineno = t_numeric_get(toks);
    program_set_next_lineno(lineno);
    return ERROR_NOERROR;
}

const f_interpreter_t keyword_funcs[NUM_KEYWORDS] =
{
    do_print,
    do_list,
    do_new,
    do_run,
    do_end,
    do_goto
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
            puts(keywords[i].str);
            break;
        }
    }

    putchar(' ');

    return toks;
}
