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
#include "t_func.h"

/* Tokenize a substring of a statement,
 * returning a pointer to the next byte to be filled
 * in the token stream.
 */
static error_t statement_tokenize_string(tok_t ** dst_ptr, const char ** input_ptr)
{
    /* Remark (comment)? */
    if (t_rem_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

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

    /* Function? */
    if (t_func_parse(dst_ptr, input_ptr)) return ERROR_NOERROR;

    /* Variable?
     * This has to come at the end so that keywords/functions
     * don't get parsed as variable names.
     */
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
    const tok_t * start = stmt;
    
    while (*stmt != TOK_TERMINATOR)
    {
        tok_t tok = *stmt;
        switch(tok)
        {
            case TOK_NUMERIC:
                stmt += NUMERIC_SIZE;
                break;

            case TOK_KEYWORD:
                stmt += KW_SIZE;
                break;
            case TOK_OPERATOR:
                stmt += OP_SIZE;
                break;
            case TOK_FUNC:
                stmt += FUNC_SIZE;
                break;
            case TOK_REGISTER:
                stmt += REGISTER_SIZE;
                break;

            case TOK_TERMINATOR:
            case TOK_SEPARATOR:
                stmt++;
                break;
            
            case TOK_STRING:
            case TOK_VARIABLE:
            case TOK_ALLOC:
            case TOK_REMARK:
                stmt = t_varlen_skip(stmt);
                break;
        }
    }

    stmt++;

    return (tok_size_t)(stmt - start);
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

        /* Keyword, so interpret the keyword. */
        kw_code kw = *stmt;
        stmt++;
        return t_keyword_interpret(kw, stmt);
    }
    else if (tok_type == TOK_VARIABLE || tok_type == TOK_REGISTER)
    {
        /* Get pointer to the variable to assign. */
        const tok_t * next_toks;
        numeric_t * val_ptr;
        error = t_variable_get_ptr(stmt, &next_toks, &val_ptr);

        /* If error is due to an undefined variable, define the variable
         * and try again. */
        if (error == ERROR_UNDEFINED_VAR)
        {
            error = program_set_numeric(stmt, 0);
            ERROR_HANDLE(error);

            error = t_variable_get_ptr(stmt, &next_toks, &val_ptr);
        }

        ERROR_HANDLE(error);

        /* Check syntax of tokens after the variable reference. */
        stmt = next_toks;
        if (!OP_CHECK(stmt, OP_EQUAL)) return ERROR_SYNTAX;
        stmt += OP_SIZE;

        /* Now we have a statement to evaluate. */
        numeric_t val;
        const tok_t * end;
        error = eval_numeric(&val, stmt, &end);
        ERROR_HANDLE(error);

        /* Eval was successful, assign the value. */
        *val_ptr = val;
        return ERROR_NOERROR;
    }
    else if (tok_type == TOK_REMARK)
    {
        /* Do nothing, just a comment. */
        return ERROR_NOERROR;
    }

    /* Not a valid beginning to an executable statement. */
    return ERROR_SYNTAX;
}
