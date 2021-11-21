#include <stdio.h>

#include "t_defs.h"

#include "t_keyword.h"
#include "t_string.h"
#include "t_numeric.h"
#include "t_operator.h"
#include "t_variable.h"

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

const tok_t * t_terminator_list(const tok_t * toks)
{
    puts("\r\n");
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

tok_size_t t_string_size(const tok_t * toks)
{
    tok_size_t size = *toks;

    /* string size + 1 for size byte. */
    return size + 1;
}

t_list_t t_list[NUM_TOKS] =
{
    t_terminator_list,
    t_string_list,
    t_keyword_list,
    t_numeric_list,
    t_operator_list,
    t_variable_list
};

const tok_t * t_defs_list(const tok_t * toks)
{
    tok_t tok_type = *toks;
    toks++;

    return t_list[tok_type](toks);
}

t_size_t t_size[NUM_TOKS] =
{
    t_terminator_size,
    t_string_size,
    t_keyword_size,
    t_numeric_size,
    t_operator_size,
    t_variable_size
};

tok_size_t t_defs_size(const tok_t * toks)
{
    tok_t tok_type = *toks;
    toks++;

    return 1 + t_size[tok_type](toks);
}
