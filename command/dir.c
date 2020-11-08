#include <stdio.h>
#include <syscall.h>
#include <string.h>

#include "dir.h"

extern char temp[512];

int command_dir(char ** argv, size_t argc)
{
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
            if (temp[f] == 0) return 0;

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

            puts(filename);
            for (uint8_t i = strlen(filename); i < 20; i++) putchar(' ');

            FINFO finfo;

            syscall_finfo(filename, &finfo);
            
            if (finfo.attr & FATTR_SYS)   putchar('s');
            else                          putchar('~');

            if (finfo.attr & FATTR_HID)   putchar('h');
            else                          putchar('~');

            if (finfo.attr & FATTR_RO)    putchar('r');
            else                          putchar('~');

            printf("  %5u", (uint16_t)finfo.size); /* Won't handle files more than 65536 in size. */

            printf("  %04u-%02u-%02u\n\r", finfo.created_year, (uint16_t)finfo.created_month, (uint16_t)finfo.created_day);
        }

        sector++;
    }
}
