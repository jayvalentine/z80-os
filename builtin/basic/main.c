#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "statement.h"
#include "errors.h"
#include "program.h"

char input[256];
uint8_t statement[256];

int user_main(char ** argv, size_t argc)
{
    error_t error;
    
    program_new();
    uint16_t free = program_free();

    puts("ZEBRA BASIC.\r\n");
    printf("%u bytes free.\r\n", free);

    while (1)
    {
        puts("> ");
        gets(input);

        error = statement_tokenize(statement, input);
        if (error != ERROR_NOERROR)
        {
            puts("T: ");
            error_display(error);
            continue;
        }

        error_t error = statement_interpret(statement);
        if (error != ERROR_NOERROR)
        {
            puts("I: ");
            error_display(error);
            continue;
        }
    }
}
