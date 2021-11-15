#include <stdio.h>

#include "program.h"

#include "statement.h"

#include "t_defs.h"
#include "t_numeric.h"

extern uint8_t _tail;

#define program_start (&_tail)
#define program_max ((uint8_t *)0xe000)

uint8_t * program_end_ptr;
const uint8_t * program_stmt_ptr;
int current_lineno;
int next_lineno;

#define PROGSTATE_RUNNING 0
#define PROGSTATE_READY 1

uint8_t program_state;

/* program_end
 *
 * Purpose:
 *     End the currently-running program.
 * 
 * Parameters:
 *     Error, if program has been ended with error.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_end(error_t error)
{
    if (program_state != PROGSTATE_RUNNING) return ERROR_NOT_RUNNING;
    program_state = PROGSTATE_READY;
    return error;
}

uint16_t program_free(void)
{
    return (uint16_t)(program_max - program_end_ptr);
}

void program_new(void)
{
    current_lineno = 0;
    program_end_ptr = program_start;
    program_state = PROGSTATE_READY;
}

error_t program_insert(const uint8_t * toks)
{
    while (*toks != TOK_TERMINATOR)
    {
        uint8_t size = t_defs_size(toks);
        for (uint8_t i = 0; i < size; i++)
        {
            *program_end_ptr = *toks;
            program_end_ptr++;
            toks++;
        }
    }

    *program_end_ptr = TOK_TERMINATOR;
    program_end_ptr++;

    return ERROR_NOERROR;
}

void program_list(void)
{
    uint8_t * p = program_start;

    while (p != program_end_ptr)
    {
        p = t_defs_list(p);
    }
}

/* Helper function to search for a statement
 * with a given linenumber.
 * Returns NULL if it can't be found.
 */
const uint8_t * program_search_lineno(int lineno, const uint8_t * stmt)
{
    const uint8_t * ptr = stmt;
    while (ptr < program_end_ptr)
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

/* Helper function to return a pointer to the statement
 * with a given line number, or the nearest higher statement
 * if none equal exists.
 */
const uint8_t * program_getlineno(int lineno)
{
    /* Greater than current lineno?
     * If so search forward from current program pointer.
     */
    if (lineno >= current_lineno)
    {
        return program_search_lineno(lineno, program_stmt_ptr);
    }

    /* Otherwise search forward from beginning
     * (can't search backwards)
     */
    return program_search_lineno(lineno, program_start);
}

error_t program_run(void)
{
    program_state = PROGSTATE_RUNNING;
    program_stmt_ptr = program_start;

    while (1)
    {
        const uint8_t * stmt = program_stmt_ptr;

        /* Sanity check, should start with a line number. */
        if (*stmt != TOK_NUMERIC)
        {
            return program_end(ERROR_LINENUM);
        }

        stmt++;

        /* Get numeric line number. */
        current_lineno = t_numeric_get(stmt);
        next_lineno = current_lineno + 1;

        /* Skip initial lineno. */
        stmt += 2;

        /* Interpret the statement. */
        error_t e = statement_interpret(stmt);

        /* We could have ended the program. */
        if (program_state != PROGSTATE_RUNNING) break;

        if (e != ERROR_NOERROR) return program_end(e);

        /* Move onto next statement. */
        program_stmt_ptr = program_getlineno(next_lineno);
        if (program_stmt_ptr == NULL) return program_end(ERROR_GOTO);
    }

    program_state = PROGSTATE_READY;
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

/* program_set_next_lineno
 *
 * Purpose:
 *     Set the next line number to be executed.
 * 
 * Parameters:
 *     Integer line number.
 * 
 * Returns:
 *     Nothing.
 */
void program_set_next_lineno(int lineno)
{
    next_lineno = lineno;
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
