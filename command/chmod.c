
#include <stdio.h>
#include <stddef.h>
#include <syscall.h>
#include <string.h>

#include "utils.h"

#include "chmod.h"

extern char temp[512];

int command_chmod(char ** argv, size_t argc)
{
    /* We expect at least two arguments - a filename and a permissions set. */
    if (argc < 2)
    {
        puts ("Usage: chmod <file> <permissions>\n\r");
        return 1;
    }

    /* File in upper-case. */
    utils_toupper(argv[0]);

    /* Attempt to find the file. */
    char filename[13];

    const DiskInfo_T * dinfo = syscall_dinfo();

    uint32_t sector = dinfo->root_region;

    uint8_t done = 0;
    while (!done)
    {
        /* Read this sector of the root directory. */
        syscall_dread(temp, sector);

        for (size_t f = 0; f < 512; f += 32)
        {
            /* If the first byte of the file is 0, stop the search. */
            if (temp[f] == 0)
            {
                printf("File '%s' does not exist.\n\r", argv[0]);
                return 2;
            }

            /* If the first byte is e5, this entry is free, so we should skip it. */
            if (temp[f] == 0xe5) continue;

            /* Check that this is a file. Skip if not. */
            uint8_t attr = temp[f+11];
            if (attr & 0b00011000) continue;

            /* Construct the filename. */
            memcpy(&filename[0], &temp[f], 8);
            
            size_t sep_pos;
            for (sep_pos = 0; sep_pos < 8; sep_pos++)
            {
                if (filename[sep_pos] == ' ') break;
            }

            size_t ext_len;
            for (ext_len = 0; ext_len < 3; ext_len++)
            {
                if (temp[f+8+ext_len] == ' ') break;
            }

            /* If no extension, don't insert the '.' */
            if (ext_len == 0)
            {
                filename[sep_pos] = '\0';
            }
            else
            {
                filename[sep_pos] = '.';
                memcpy(&filename[sep_pos+1], &temp[f+8], ext_len);
                filename[sep_pos+1+ext_len] = '\0';
            }

            if (strcmp(filename, argv[0]) == 0)
            {
                /* Check the attributes. */
                if (strchr(argv[1], 's') == 0)  attr &= ~FATTR_SYS;
                else                            attr |= FATTR_SYS;

                if (strchr(argv[1], 'h') == 0)  attr &= ~FATTR_HID;
                else                            attr |= FATTR_HID;

                if (strchr(argv[1], 'r') == 0)  attr &= ~FATTR_RO;
                else                            attr |= FATTR_RO;

                /* Set attribute byte on disk. */
                temp[f+11] = attr;
                syscall_dwrite(temp, sector);

                /* And we're done! */
                puts("Done.\n\r");
                return 0;
            }
        }

        sector++;
    }
}
