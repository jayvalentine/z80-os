#include <stdio.h>

#include "t_string.h"

/* t_string_parse
 *
 * Purpose:
 *     Parse a string in the given input into the token stream.
 *
 * Parameters:
 *     stmt:  Token stream to write to.
 *     input: Input from which to parse string.
 * 
 * Returns:
 *     Pointer to the next byte after the tokenized string
 *     in the token stream.
 */
int t_string_parse(uint8_t ** dst_ptr, const char ** input_ptr)
{
    const char * input = *input_ptr;
    uint8_t * dst = *dst_ptr;

    if (*input != '"') return 0;

    *dst = TOK_STRING;
    dst++;

    /* Can't set size right now. */
    uint8_t * size = dst;
    dst++;

    /* Skip initial '"' */
    input++;

    uint8_t str_size = 0;
    while (*input != '"')
    {
        *dst = *input;
        dst++; input++;
        str_size++;
    }

    *size = str_size;

    /* Skip final '"' */
    *input++;

    /* Update pointers. */
    *input_ptr = input;
    *dst_ptr = dst;

    return 1;
}

const uint8_t * t_string_list(const uint8_t * toks)
{
    putchar('"');

    uint8_t size = *toks;
    toks++;

    for (uint8_t i = 0; i < size; i++)
    {
        char c = *toks;
        toks++;
        putchar(c);
    }

    puts("\" ");
    return toks;
}
