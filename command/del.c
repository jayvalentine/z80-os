#include <stdio.h>
#include <syscall.h>
#include <string.h>

#include "del.h"

int command_del(char ** argv, size_t argc)
{
    if (argc != 1)
    {
        puts("Usage: del <filename>\n\r");
        return 1;
    }

    /* Get filename - first argument. */
    const char * filename = argv[0];
    
    /* Delete the file, checking for any errors. */
    int error = syscall_fdelete(filename);

    if (error == E_FILENOTFOUND)
    {
        puts("File not found.\n\r");
        return 2;
    }
    else if (error == 0)
    {
        puts("Deleted.\n\r");
    }
    else
    {
        printf("Unknown error during deletion: %u\n\r", error);
    }

    return 0;
}
