/* Z80-OS Kernel.
 * Copyright (C) 2020 Jay Valentine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <syscall.h>

#include "file.h"

char input[256];

/* Entry point for command-processor. */
typedef void (*cp_main_t)(void);
#define cp_main ((cp_main_t)0x6000)

void main(void)
{
    puts("Initialising kernel... ");

    filesystem_init();

    puts("Done.\n\r");

    puts("Loading command processor... ");

    /* Load command processor into the last 8k of low-RAM. */
    void * cp_addr = 0x6000;

    int fd = syscall_fopen("COMMAND.BIN", FMODE_READ);

    if (fd == E_FILENOTFOUND)
    {
        puts("Error: COMMAND.BIN not found.\n\r");
    }
    else
    {
        size_t bytes = syscall_fread(cp_addr, 0x2000, fd);
        syscall_fclose(fd);

        uint8_t bytes_hi = bytes >> 8;
        uint8_t bytes_lo = bytes & 0x00ff;

        printf("Read 0x%x%x bytes.\n\r", bytes_hi, bytes_lo);

        cp_main();
    }
}
