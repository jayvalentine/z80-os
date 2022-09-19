#include <stdio.h>
#include <syscall.h>
#include <string.h>

#include "dir.h"

int command_dir(char ** argv, size_t argc)
{
    argv; argc;

    char filename[13];
    FINFO finfo;

    uint16_t file_entries = syscall_fentries();

    printf("%u files:\n\r", file_entries);

    for (uint16_t f = 0; f < file_entries; f++)
    {
        int error = syscall_fentry(&filename[0], f);
        if (error == 0)
        {
            puts("    ");
            puts(&filename[0]);
            for (uint8_t i = strlen(&filename[0]); i < 20; i++) putchar(' ');

            syscall_finfo(&filename[0], &finfo);
            
            if (finfo.attr & FATTR_SYS)   putchar('s');
            else                          putchar('~');

            if (finfo.attr & FATTR_HID)   putchar('h');
            else                          putchar('~');

            if (finfo.attr & FATTR_RO)    putchar('r');
            else                          putchar('~');

            printf("  %5u", (uint16_t)finfo.size); /* Won't handle files more than 65536 in size. */

            printf("  %04u-%02u-%02u\n\r", finfo.created_year, (uint16_t)finfo.created_month, (uint16_t)finfo.created_day);
        }
        else
        {
            printf("    Error in file entry %u: %d\n\r", f, error);
        }
    }

    return 0;
}
