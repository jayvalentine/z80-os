#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "statement.h"
#include "keyword.h"

/* Tokenize a substring of a statement,
 * returning a pointer to the next byte to be filled
 * in the token stream.
 */
static uint8_t * statement_tokenize_string(uint8_t * dst, const char * input)
{
    kw_code keyword = keyword_parse(input);

    if (keyword == KEYWORD_UNDEFINED)
    {
        *dst = 0x0f;
    }
    else
    {
        *dst = keyword;
    }

    return dst + 1; 
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
    const char * s;

    /* Expect at least one substring. */
    s = strtok(input, " ");

    stmt = statement_tokenize_string(stmt, s);

    while (1)
    {
        s = strtok(NULL, " ");

        if (s == NULL) break;

        stmt = statement_tokenize_string(stmt, s);
    }

    /* Final terminating character. */
    *stmt = 0;
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
    while (*stmt != 0)
    {
        uint16_t tok = *stmt;
        printf("%x ", tok);
        stmt++;
    }

    puts("\r\n");
}
