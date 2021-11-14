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

    puts("IS STRING\r\n");

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

    printf("SIZE: %x\r\n", str_size);

    *size = str_size;

    /* Skip final '"' */
    *input++;

    printf("DONE AT %x (stmt %x)\r\n", (uint16_t)input, (uint16_t)dst);

    /* Update pointers. */
    *input_ptr = input;
    *dst_ptr = dst;

    return 1;
}
