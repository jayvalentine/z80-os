#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "t_defs.h"

#include "statement.h"
#include "errors.h"
#include "program.h"

char input[256];
tok_t statement[256];

/* If we're compiling for Z80
 * (i.e. we're compiling to run on the desktop for testing)
 * then we need to make sure the code will compile.
 */
#ifdef Z80
#include <file.h>

FILE thefile;

/* NASTY!!! */
#define main(_argc, _argv) user_main(_argv, _argc)
#define ARG_LIM 1
#define ARG_FIRST 0

#else
#define ARG_LIM 2
#define ARG_FIRST 1
#endif

error_t process_line(void)
{
    error_t error;

    error = statement_tokenize(statement, input);
    if (error != ERROR_NOERROR)
    {
        printf("TOK: ");
        printf("L%u ", program_current_lineno());
        error_display(error);
        return error;
    }

    error = statement_interpret(statement);
    if (error != ERROR_NOERROR)
    {
        printf("INT: ");
        printf("L%d ", program_current_lineno());
        error_display(error);
        return error;
    }

    return ERROR_NOERROR;
}

void srand16(uint16_t seed);

int main(size_t argc, char ** argv)
{
    srand16(1);
    program_new();
    
    /* If an argument has been provided, assume it's a file
     * and read it into the program.
     */
    if (argc == ARG_LIM)
    {
        printf("Reading %s...", argv[ARG_FIRST]);
        FILE * const f = fopen(argv[ARG_FIRST], "r");
        while (1)
        {
            char * s = fgets(input, 256, f);
            if (s != input) break;

            /* Sanitise input (removing trailing newline). */
            input[strcspn(input, "\r\n")] = '\0';
            
            /* Skip blank line. */
            if (strlen(input) == 0) continue;

            if (process_line() != ERROR_NOERROR)
            {
                break;
            }

            putchar('.');
        }

        putchar('!');
        fclose(f);
        printf(" done.\r\n");
    }

    uint16_t free = program_free();

    puts("ZEBRA BASIC.\r\n");
    printf("%u bytes free.\r\n", free);

    while (1)
    {
        printf("> ");
        gets(input);

        process_line();
    }
}
