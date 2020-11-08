#include <syscall.h>
#include <stdio.h>

#include "type.h"

int command_type(char ** argv, size_t argc)
{
    char type_buf[17];

    if (argc < 1)
    {
        puts("Usage: type <files>\n\r");
        return 1;
    }

    for (size_t a = 0; a < argc; a++)
    {
        int fd = syscall_fopen(argv[a], FMODE_READ);

        if (fd == E_FILENOTFOUND)
        {
            printf("Could not find file: %s\n\r", argv[a]);
            return 2;
        }

        size_t bytes;

        /* Print the contents of the file. */
        while ((bytes = syscall_fread(type_buf, 16, fd)) > 0)
        {
            type_buf[bytes] = '\0';
            puts(type_buf);
        }

        syscall_fclose(fd);
    }

    return 0;
}