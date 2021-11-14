#include <stdio.h>

#include "t_numeric.h"

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
int t_numeric_parse(uint8_t ** dst_ptr, const char ** input_ptr)
{
    const char * input = *input_ptr;
    uint8_t * dst = *dst_ptr;

    if (*input != '-' && (*input < '0' || *input > '9'))
    {
        return 0;
    }

    char sign = ' ';
    if (*input == '-')
    {
        sign = '-';
        input++;
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

    int val = first_digit - '0';

    while (*input >= '0' && *input <= '9')
    {
        val = val * 10;

        char digit = *input;
        input++;
        val += (digit - '0');
    }

    /* Sign? */
    if (sign == '-')
    {
        val = -val;
    }

    int * numeric_ptr = (int *)dst;
    *numeric_ptr = val;
    dst += 2;

    /* Update pointers. */
    *input_ptr = input;
    *dst_ptr = dst;

    return 1;
}

const uint8_t * t_numeric_list(const uint8_t * toks)
{
    int * numeric_ptr = (int *)toks;
    toks += 2;

    int val = *numeric_ptr;

    char sign = (val < 0) ? '-' : ' ';

    /* Make absolute. */
    if (val < 0) val = -val;

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

    if (sign != ' ')          putchar(sign);
    if (ten_thousands != '0') putchar(ten_thousands);
    if (thousands != '0')     putchar(thousands);
    if (hundreds != '0')      putchar(hundreds);
    if (tens != '0')          putchar(tens);
    putchar(units);
    putchar(' ');

    return toks;
}