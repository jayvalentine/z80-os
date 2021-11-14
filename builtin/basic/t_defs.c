#include <stdio.h>

#include "t_defs.h"

#include "t_keyword.h"
#include "t_string.h"
#include "t_numeric.h"

const uint8_t * t_terminator_list(const uint8_t * toks)
{
    puts("\r\n");
    return toks;
}

uint8_t t_terminator_size(const uint8_t * toks)
{
    return 0;
}

uint8_t t_keyword_size(const uint8_t * toks)
{
    return 1;
}

uint8_t t_string_size(const uint8_t * toks)
{
    uint8_t size = *toks;

    /* string size + 1 for size byte. */
    return size + 1;
}

uint8_t t_numeric_size(const uint8_t * toks)
{
    return 2;
}

t_list_t t_list[NUM_TOKS] =
{
    t_terminator_list,
    t_string_list,
    t_keyword_list,
    t_numeric_list
};

uint8_t * t_defs_list(const uint8_t * toks)
{
    uint8_t tok_type = *toks;
    toks++;

    return t_list[tok_type](toks);
}

t_size_t t_size[NUM_TOKS] =
{
    t_terminator_size,
    t_string_size,
    t_keyword_size,
    t_numeric_size
};

uint8_t t_defs_size(const uint8_t * toks)
{
    uint8_t tok_type = *toks;
    toks++;

    return 1 + t_size[tok_type](toks);
}
