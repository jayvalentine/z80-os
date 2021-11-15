#include <stdio.h>

#include "program.h"

#include "statement.h"

#include "t_defs.h"
#include "t_numeric.h"

extern uint8_t _tail;

#define program_start (&_tail)
#define program_max ((uint8_t *)0xe000)

uint8_t * program_end;
const uint8_t * program_stmt_ptr;
int current_lineno;
int next_lineno;

uint16_t program_free(void)
{
    return (uint16_t)(program_max - program_end);
}

void program_new(void)
{
    current_lineno = 0;
    program_end = program_start;
}

error_t program_insert(const uint8_t * toks)
{
    while (*toks != TOK_TERMINATOR)
    {
        uint8_t size = t_defs_size(toks);
        for (uint8_t i = 0; i < size; i++)
        {
            *program_end = *toks;
            program_end++;
            toks++;
        }
    }

    *program_end = TOK_TERMINATOR;
    program_end++;

    return ERROR_NOERROR;
}

void program_list(void)
{
    uint8_t * p = program_start;

    while (p != program_end)
    {
        p = t_defs_list(p);
    }
}

/* Helper function to return a pointer to the statement
 * with a given line number, or the nearest higher statement
 * if none equal exists.
 */
const uint8_t * program_getlineno(int lineno)
{
    /* Greater than current lineno?
     * If so increment from current program pointer.
     */
    if (lineno >= current_lineno)
    {
        const uint8_t * ptr = program_stmt_ptr;
        while (ptr < program_end)
        {
            /* Get numeric. */
            uint8_t tok_type = *ptr;
            if (tok_type != TOK_NUMERIC) return NULL;

            int this_lineno = t_numeric_get(ptr+1);
            if (this_lineno >= lineno)
            {
                return ptr;
            }

            /* Otherwise not matching, move onto next statement. */
            ptr += statement_size(ptr);
        }

        /* Didn't find matching line number. */
        return NULL;
    }

    /* Otherwise search from beginning
     * (can't search backwards)
     */
    return NULL;
}

error_t program_run(void)
{
    program_stmt_ptr = program_start;

    while (program_stmt_ptr < program_end)
    {
        const uint8_t * stmt = program_stmt_ptr;

        /* Sanity check, should start with a line number. */
        if (*stmt != TOK_NUMERIC)
        {
            return ERROR_LINENUM;
        }

        stmt++;

        /* Get numeric line number. */
        current_lineno = t_numeric_get(stmt);
        next_lineno = current_lineno + 1;

        /* Skip initial lineno. */
        stmt += 2;

        /* Interpret the statement. */
        error_t e = statement_interpret(stmt);
        if (e != ERROR_NOERROR) return e;

        /* Move onto next statement. */
        program_stmt_ptr = program_getlineno(next_lineno);
        if (program_stmt_ptr == NULL) return ERROR_GOTO;
    }

    return ERROR_NOERROR;
}

/* program_current_lineno
 *
 * Purpose:
 *     Get the current line number of the executing program.
 *
 * Parameters:
 *     Nothing.
 *
 * Returns:
 *     Integer line number.
 */
int program_current_lineno(void)
{
    return current_lineno;
}

/* program_next_lineno
 *
 * Purpose:
 *     Get the next line number to be executed.
 *     Mainly used for error reporting.
 * 
 * Parameters:
 *     Nothing.
 * 
 * Returns:
 *     Integer line number.
 */
int program_next_lineno(void)
{
    return next_lineno;
}
