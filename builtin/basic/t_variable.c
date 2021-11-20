#include "t_variable.h"

#include <stdio.h>

/* t_variable_parse
 *
 * Purpose:
 *     Parse a variable from the input stream.
 * 
 * Parameters:
 *     dst_ptr:   Reference to pointer in destination stream.
 *     input_ptr: Reference to pointer in input stream.
 * 
 * Returns:
 *     1 if successful, 0 otherwise.
 */
int t_variable_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    tok_t * dst = *dst_ptr;
    const char * input = *input_ptr;

    *dst = TOK_VARIABLE;
    dst++;

    /* Can't set size yet. */
    tok_t * size_ptr = dst;
    dst++;

    tok_size_t size = 0;

    while (*input >= 'A' && *input <= 'Z')
    {
        if (size < VARNAME_SIZE)
        {
            *dst = *input;
            dst++;
            size++;
        }

        input++;
    }

    if (size == 0)
    {
        return 0;
    }

    *size_ptr = size;
    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
}

/* t_variable_size
 *
 * Purpose:
 *     Get the size of a variable token.
 * 
 * Parameters:
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Size of variable token.
 */
tok_size_t t_variable_size(const tok_t * toks)
{
    tok_size_t size = *toks;

    /* string size + 1 for size byte. */
    return size + 1;
}

/* t_variable_list
 *
 * Purpose:
 *     Print an variable token.
 * 
 * Parameters:
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Pointer to token after the variable.
 */
const tok_t * t_variable_list(const tok_t * toks)
{
    uint8_t size = *toks;
    toks++;

    for (uint8_t i = 0; i < size; i++)
    {
        char c = *toks;
        toks++;
        putchar(c);
    }
    
    return toks;
}
