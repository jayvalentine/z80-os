#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "statement.h"
#include "eval.h"
#include "program.h"

#include "t_defs.h"
#include "t_string.h"
#include "t_keyword.h"
#include "t_numeric.h"
#include "t_operator.h"
#include "t_variable.h"

/* Tokenize a substring of a statement,
 * returning a pointer to the next byte to be filled
 * in the token stream.
 */
static error_t statement_tokenize_string(tok_t ** dst_ptr, const char ** input_ptr)
{
    /* Separator? */
    if (t_sep_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;
    
    /* Is this a string? */
    if (t_string_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

    /* Operator? */
    if (t_operator_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

    /* Is it numeric? */
    if (t_numeric_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

    /* Keyword? */
    if (t_keyword_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

    /* Variable? */
    if (t_variable_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

    return ERROR_SYNTAX;
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
error_t statement_tokenize(tok_t * stmt, const char * input)
{
    error_t error;

    error = statement_tokenize_string(&stmt, &input);

    if (error != ERROR_NOERROR) return error;

    while (1)
    {
        while (*input == ' ') input++;
        if (*input == '\0') break;

        error = statement_tokenize_string(&stmt, &input);

        if (error != ERROR_NOERROR) return error;
    }

    /* Final terminating character. */
    *stmt = TOK_TERMINATOR;

    return ERROR_NOERROR;
}

tok_size_t statement_size(const tok_t * stmt)
{
    tok_size_t size = 0;
    while (*stmt != TOK_TERMINATOR)
    {
        tok_size_t tok_size = t_defs_size(stmt);
        size += tok_size;
        stmt += tok_size;
    }

    /* Account for final terminator. */
    size++;
    return size;
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
error_t statement_interpret(const tok_t * stmt)
{
#ifdef DEBUG
    uint8_t * p = stmt;
    while (*p != TOK_TERMINATOR)
    {
        uint8_t size = t_defs_size(p);
        for (uint8_t i = 0; i < size; i++)
        {
            uint16_t tok = *p;
            printf("%x ", tok);
            p++;
        }

        puts("! ");
    }
    puts("\r\n");
#endif

    tok_t tok_type = *stmt;
    error_t error = ERROR_NOERROR;

    /* We've got a line in the program. */
    if (tok_type == TOK_NUMERIC)
    {
        return program_insert(stmt);
    }

    /* Otherwise it's something we're interpreting directly. */
    if (tok_type == TOK_KEYWORD)
    {
        stmt++;
        kw_code kw = *stmt;
        stmt++;
        return t_keyword_interpret(kw, stmt);
    }
    else if (tok_type == TOK_VARIABLE)
    {
        stmt++;

        /* Assignment. Check that the next token is equals. */
        char varname[VARNAME_BUF_SIZE];
        t_variable_get(varname, stmt);

        stmt += t_variable_size(stmt);
        if (*stmt != TOK_OPERATOR) return ERROR_SYNTAX;
        stmt++;
        if (*stmt != OP_EQUAL) return ERROR_SYNTAX;
        stmt++;

        /* Now we have a statement to evaluate. */
        numeric_t val;
        error_t error = eval_numeric(&val, stmt);
        if (error != ERROR_NOERROR) return error;

        /* Eval was successful, assign the value. */
        return program_set_numeric(varname, val);
    }
}
