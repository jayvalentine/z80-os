#include <syscall.h>
#include <stdio.h>

#include "type.h"

#define TYPE_BUF_SIZE 128

int command_type(char ** argv, size_t argc)
{
    char type_buf[TYPE_BUF_SIZE+1];

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
        while ((bytes = syscall_fread(type_buf, TYPE_BUF_SIZE, fd)) > 0)
        {
            /* Get location of first SUB */
            size_t end_index;
            for (end_index = 0; end_index < bytes; end_index++)
            {
                if (type_buf[end_index] == 0x1a) break;
            }

            type_buf[end_index] = '\0';
            puts(type_buf);

            for (int i = 0; i < 2000; i++);
        }

        syscall_fclose(fd);
    }

    return 0;
}