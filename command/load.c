#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

jmp_buf env;

void cancel_handler(uint16_t val)
{
    longjmp(env, 1);
}

int command_load(char ** argv, size_t argc)
{
    if (argc < 1)
    {
        puts("Usage: LOAD <filename>\n\r");
        return 1;
    }

    char * filename = argv[0];
    int fd = syscall_fopen(filename, FMODE_WRITE);

    if (fd != 0)
    {
        if (fd == E_FILEEXIST)
        {
            printf("File %s already exists.\n\r", filename);
        }
        else
        {
            puts("Unknown error while opening file.\n\r");
        }

        return 2;
    }

    /* Set up handler. */
    syscall_sighandle(cancel_handler, SIG_CANCEL);

    size_t bytes_total = 0;
    char temp[128];

    while (1)
    {
        int val = setjmp(env);
        if (val != 0) break;

        gets(temp);
        if (strlen(temp) == 0) break;
        
        memcpy(temp + strlen(temp), "\n\r", 3);

        size_t bytes = syscall_fwrite(temp, strlen(temp), fd);

        if (bytes == 0)
        {
            puts("Error writing file.\n\r");
            syscall_fclose(fd);
            return 3;
        }

        bytes_total += bytes;
    }

    char term = '\0';

    syscall_fwrite(&term, 1, fd);

    /* Exit due to cancel. */
    syscall_fclose(fd);
    printf("Wrote %u bytes.\n\r", bytes_total);
    return 0;
}
