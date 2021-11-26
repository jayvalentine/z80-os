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

#include <include/status.h>
#include <include/file.h>
#include <include/signal.h>

char input[256];

/* Entry point for command-processor. */
typedef void (*cp_main_t)(void);
#define cp_main ((cp_main_t)0x6000)

uint8_t startup_flags = 0;

void main(void)
{
    status_init();
    
#ifndef DEBUG
    puts("Initialising kernel... ");
#endif

    filesystem_init();
    signal_init();

#ifdef DEBUG
    /* Just call the program in the command processor memory
     * directly, it will have been loaded by the test.
     */
    cp_main();
#else
    puts("Done.\n\r");
    puts("Loading command processor... ");

    /* Load command processor into the last 8k of low-RAM. */
    void * cp_addr = 0x6000;

    int fd = syscall_fopen("COMMAND.BIN", FMODE_READ);

    if (fd == E_FILENOTFOUND)
    {
        puts("Error: COMMAND.BIN not found.\n\r");
        return;
    }

    if (fd < 0)
    {
        puts("Unknown error opening COMMAND.BIN.\n\r");
        return;
    }

    size_t bytes = syscall_fread(cp_addr, 0x2000, fd);
    syscall_fclose(fd);

    if (bytes == 0)
    {
        puts("Error reading COMMAND.BIN.\n\r");
        return;
    }

    printf("Read %u bytes.\n\r", bytes);
    cp_main();
#endif
}
