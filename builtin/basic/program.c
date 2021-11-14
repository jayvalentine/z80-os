#include <stdio.h>

#include "program.h"

#include "statement.h"

#include "t_defs.h"

extern uint8_t _tail;

#define program_start (&_tail)
#define program_max ((uint8_t *)0xe000)

uint8_t * program_end;

uint16_t program_free(void)
{
    return (uint16_t)(program_max - program_end);
}

void program_new(void)
{
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

uint8_t * program_stmt_ptr;

error_t program_run(void)
{
    program_stmt_ptr = program_start;

    while (program_stmt_ptr != program_end)
    {
        /* Sanity check, should start with a line number. */
        if (*program_stmt_ptr != TOK_NUMERIC)
        {
            return ERROR_LINENUM;
        }

        /* Skip initial lineno. */
        program_stmt_ptr += 3;

        uint8_t size = statement_size(program_stmt_ptr);

        /* Interpret the statement. */
        error_t e = statement_interpret(program_stmt_ptr);
        if (e != ERROR_NOERROR) return e;

        /* Move onto next statement. */
        program_stmt_ptr += size;
    }

    return ERROR_NOERROR;
}
