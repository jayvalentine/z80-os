#include <stdio.h>

#include "t_defs.h"

#include "t_keyword.h"
#include "t_string.h"
#include "t_numeric.h"
#include "t_operator.h"
#include "t_variable.h"
#include "t_func.h"

/* Parse a separator from the input stream. */
int t_sep_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    tok_t * dst = *dst_ptr;
    const char * input = *input_ptr;

    if (*input != ',') return 0;

    *dst = TOK_SEPARATOR;
    dst++; input++;

    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
}

int t_rem_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    const char * input = *input_ptr;

    /* Check first three characters of input. */
    if (input[0] != 'R' || input[1] != 'E' || input[2] != 'M')
    {
        return 0;
    }

    input += 3;

    tok_t * dst = *dst_ptr;
    *dst = TOK_REMARK;
    dst++;

    tok_t * size_ptr = dst;
    dst++;

    *size_ptr = 0;

    /* Get characters of remark string. */
    while (*input != '\0')
    {
        *dst = *input;
        dst++; input++;

        *size_ptr += 1;
    }

    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
}

const tok_t * t_terminator_list(const tok_t * toks)
{
    putchar('\r');
    putchar('\n');
    return toks;
}

const tok_t * t_separator_list(const tok_t * toks)
{
    printf(", ");
    return toks;
}

const tok_t * t_alloc_list(const tok_t * toks)
{
    tok_size_t size = *toks;
    return toks + size + 1;
}

const tok_t * t_rem_list(const tok_t * toks)
{
    printf("REM");

    tok_size_t size = *toks;
    toks++;
    while (size > 0)
    {
        putchar(*toks);
        toks--;
    }

    return toks;
}

tok_size_t t_terminator_size(const tok_t * toks)
{
    return 0;
}

tok_size_t t_keyword_size(const tok_t * toks)
{
    return 1;
}

tok_size_t t_separator_size(const tok_t * toks)
{
    return 0;
}

tok_size_t t_string_size(const tok_t * toks)
{
    tok_size_t size = *toks;

    /* string size + 1 for size byte. */
    return size + 1;
}

tok_size_t t_alloc_size(const tok_t * toks)
{
    tok_size_t size = *toks;

    return size + 1;
}

tok_size_t t_rem_size(const tok_t * toks)
{
    tok_size_t size = *toks;
    return size + 1;
}

const t_list_t t_list[NUM_TOKS] =
{
    t_terminator_list,
    t_string_list,
    t_keyword_list,
    t_numeric_list,
    t_operator_list,
    t_variable_list,
    t_separator_list,
    t_alloc_list,
    t_func_list,
    t_rem_list,
};

const tok_t * t_defs_list(const tok_t * toks)
{
    tok_t tok_type = *toks;
    toks++;

    return t_list[tok_type](toks);
}

const t_size_t t_size[NUM_TOKS] =
{
    t_terminator_size,
    t_string_size,
    t_keyword_size,
    t_numeric_size,
    t_operator_size,
    t_variable_size,
    t_separator_size,
    t_alloc_size,
    t_func_size,
    t_rem_size,
};

tok_size_t t_defs_size(const tok_t * toks)
{
    tok_t tok_type = *toks;
    toks++;

    return 1 + t_size[tok_type](toks);
}
