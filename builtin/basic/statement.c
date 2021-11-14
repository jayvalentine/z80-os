#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "statement.h"

#include "t_defs.h"
#include "t_string.h"
#include "t_keyword.h"

/* Tokenize a substring of a statement,
 * returning a pointer to the next byte to be filled
 * in the token stream.
 */
static void statement_tokenize_string(uint8_t ** dst_ptr, const char ** input_ptr)
{
    /* Is this a string? */
    if (t_string_parse(dst_ptr, input_ptr)) return;

    /* Otherwise it must be a keyword. */
    if (t_keyword_parse(dst_ptr, input_ptr)) return;
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
    statement_tokenize_string(&stmt, &input);

    while (1)
    {
        while (*input == ' ') input++;
        if (*input == '\0') break;

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
 *     error, if any.
 */
error_t statement_interpret(const uint8_t * stmt)
{
    uint8_t tok_type = *stmt;
    stmt++;

    error_t error = ERROR_NOERROR;

    if (tok_type == TOK_KEYWORD)
    {
        kw_code kw = *stmt;
        stmt++;
        return t_keyword_interpret(kw, stmt);
    }
}
