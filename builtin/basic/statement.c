#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "statement.h"
#include "keyword.h"

#include "t_defs.h"
#include "t_string.h"

/* Tokenize a substring of a statement,
 * returning a pointer to the next byte to be filled
 * in the token stream.
 */
static void statement_tokenize_string(uint8_t ** dst_ptr, const char ** input_ptr)
{
    /* Is this a string? */
    if (t_string_parse(dst_ptr, input_ptr)) return;

    /* Otherwise it must be a keyword. */
    if (keyword_parse(dst_ptr, input_ptr)) return;
}

/* statement_tokenize
 *
 * Purpose:
 *     Parse a statement into a sequence of tokens.
 * 
 * Parameters:
 *     stmt:  Buffer for generated token stream
 *     input: Buffer containing input to be tokenized
 * 
 * Returns:
 *     length of token stream in bytes.
 */
void statement_tokenize(uint8_t * stmt, char * input)
{
    printf("Tokenizing at %x (stmt %x)\r\n", (uint16_t)input, (uint16_t)stmt);
    statement_tokenize_string(&stmt, &input);

    while (1)
    {
        while (*input == ' ') input++;
        if (*input == '\0') break;

        printf("Tokenizing at %x (stmt %x): ", (uint16_t)input, (uint16_t)stmt);
        putchar(*input);
        puts("\r\n");

        statement_tokenize_string(&stmt, &input);
    }

    /* Final terminating character. */
    *stmt = TOK_TERMINATOR;
}

/* statement_interpret
 *
 * Purpose:
 *     Interpret the given token stream.
 * 
 * Parameters:
 *     stmt: Token stream to interpret.
 * 
 * Returns:
 *     nothing.
 */
void statement_interpret(const uint8_t * stmt)
{
    for (int i = 0; i < 16; i++)
    {
        uint16_t tok = *stmt;
        printf("%x ", tok);
        stmt++;
    }

    puts("\r\n");
}
