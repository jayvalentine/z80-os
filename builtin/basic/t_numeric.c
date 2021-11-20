#include <stdio.h>

#include "t_numeric.h"

/* t_numeric_get
 *
 * Purpose:
 *     Return a numeric (int) from the given token stream.
 * 
 * Parameters:
 *     toks: Token stream to read from.
 * 
 * Returns:
 *     integer value of numeric.
 */
numeric_t t_numeric_get(const tok_t * toks)
{
    numeric_t * numeric_ptr = (numeric_t *)toks;
    return *numeric_ptr;
}

/* t_numeric_size
 * Purpose:
 *     Return the size of a numeric in memory.
 * 
 * Parameters:
 *     toks: Token stream to read from.
 * 
 * Returns:
 *     unsigned size of numeric tokens (2).
 */

tok_size_t t_numeric_size(const tok_t * toks)
{
    return 2;
}

/* t_numeric_parse
 *
 * Purpose:
 *     Parse a numeric in the given input into the token stream.
 *
 * Parameters:
 *     stmt:  Token stream to write to.
 *     input: Input from which to parse numeric.
 * 
 * Returns:
 *     true value if a numeric has been parsed, false otherwise.
 */
int t_numeric_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    const char * input = *input_ptr;
    tok_t * dst = *dst_ptr;

    if (*input != '-' && (*input < '0' || *input > '9'))
    {
        return 0;
    }

    /* Next input needs to be digit. */
    if (*input < '0' || *input > '9')
    {
        return 0;
    }

    *dst = TOK_NUMERIC;
    dst++;

    char first_digit = *input;
    input++;

    numeric_t val = first_digit - '0';

    while (*input >= '0' && *input <= '9')
    {
        val = val * 10;

        char digit = *input;
        input++;
        val += (digit - '0');
    }

    numeric_t * numeric_ptr = (numeric_t *)dst;
    *numeric_ptr = val;
    dst += 2;

    /* Update pointers. */
    *input_ptr = input;
    *dst_ptr = dst;

    return 1;
}

const tok_t * t_numeric_list(const tok_t * toks)
{
    numeric_t * numeric_ptr = (numeric_t *)toks;
    toks += 2;

    numeric_t val = *numeric_ptr;

    char ten_thousands = '0';
    char thousands = '0';
    char hundreds = '0';
    char tens = '0';
    char units = '0';

    while (val >= 10000)
    {
        ten_thousands++;
        val -= 10000;
    }

    while (val >= 1000)
    {
        thousands++;
        val -= 1000;
    }

    while (val >= 100)
    {
        hundreds++;
        val -= 100;
    }

    while (val >= 10)
    {
        tens++;
        val -= 10;
    }

    /* Guaranteed to be <10 now. */
    units += val;

    if (ten_thousands != '0') putchar(ten_thousands);
    if (thousands != '0')     putchar(thousands);
    if (hundreds != '0')      putchar(hundreds);
    if (tens != '0')          putchar(tens);
    putchar(units);
    putchar(' ');

    return toks;
}

/* t_numeric_put
 *
 * Purpose:
 *     Write a numeric (int) to the given token stream.
 * 
 * Parameters:
 *     toks: Token stream to write to.
 *     num:  Numeric value to write.
 * 
 * Returns:
 *     Nothing.
 */
void t_numeric_put(tok_t * toks, numeric_t num)
{
    *toks = TOK_NUMERIC;
    toks++;
    numeric_t * num_ptr = (numeric_t *)toks;
    *num_ptr = num;
}
