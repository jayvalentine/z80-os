#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "statement.h"
#include "errors.h"

char input[256];
uint8_t statement[256];

extern char _tail;

#define user_program_start (&_tail)
char * user_program_end = &_tail;

int user_main(char ** argv, size_t argc)
{
    *user_program_end = '\0';

    while (1)
    {
        puts("> ");
        gets(input);

        statement_tokenize(statement, input);

        error_t error = statement_interpret(statement);
        if (error != ERROR_NOERROR)
        {
            error_display(error);
        }
    }
}
