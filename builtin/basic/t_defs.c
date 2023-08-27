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

    /* Add null-terminator. */
    *dst = '\0';
    dst++;
    *size_ptr += 1;

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

const tok_t * t_register_list(const tok_t * toks)
{
    putchar(*toks + 'A');
    return toks+1;
}

const tok_t * t_alloc_list(const tok_t * toks)
{
    tok_size_t size = *toks;
    return toks + size + 1;
}

const tok_t * t_rem_list(const tok_t * toks)
{
    printf("REM%s", (const char *)(toks+1));

    tok_size_t size = *toks;
    toks += size;

    return toks;
}

const tok_t * t_varlen_skip(const tok_t * toks)
{
    toks++;
    tok_size_t size = *toks;
    toks++;

    return toks + size;
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
    t_register_list
};

const tok_t * t_defs_list(const tok_t * toks)
{
    tok_t tok_type = *toks;
    toks++;

    return t_list[tok_type](toks);
}
