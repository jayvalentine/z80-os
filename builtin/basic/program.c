#include <stdio.h>

#include "program.h"

#include "t_defs.h"

extern uint8_t _tail;

#define program_start (&_tail)
uint8_t * program_end;

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
