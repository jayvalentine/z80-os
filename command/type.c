#include <syscall.h>
#include <stdio.h>
#include <file.h>

#include "type.h"

FILE thefile;

int command_type(char ** argv, size_t argc)
{
    if (argc < 1)
    {
        puts("Usage: type <files>\n\r");
        return 1;
    }

    for (size_t a = 0; a < argc; a++)
    {
        FILE * f = fopen(argv[a], "r");

        if (f == NULL)
        {
            printf("Could not find file: %s\n\r", argv[a]);
            return 2;
        }

        char line[256];

        /* Print the contents of the file. */
        while (fgets(line, 256, f) != NULL)
        {
            puts(line);
        }

        fclose(f);
    }

    return 0;
}