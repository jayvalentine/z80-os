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
#include <include/ram.h>
#include <include/process.h>
#include <include/memory.h>
#include <include/scheduler.h>
#include <include/serial.h>

extern SysInfo_T sysinfo;

char input[256];

uint8_t startup_flags = 0;

typedef void (*proc_t)(void);

void interrupt_enable(void);
void debug_process_run(void);

void main(void)
{
    status_init();

    filesystem_init();
    signal_init();

    serial_init();

    /* Get number of RAM banks. */
    //uint16_t banks = ram_bank_test();
    uint16_t banks = 16;
    sysinfo.numbanks = banks;
    ram_bank_set(15);

    memory_init(banks);
    process_init();

    scheduler_init();

    interrupt_enable();

#ifndef DEBUG
    /* Check startup flags. Warn the user if there is a problem. */
    if (startup_flags != 0)
    {
        printf("\r\nError Flag: %u\r\n", (uint16_t)startup_flags);
        puts("Press any key to continue.\r\n");
        getchar();
        startup_flags = 0;
    }
    else
    {
        puts("Done.\n\r");
    }

    puts("Loading command processor... ");

    /* Load command processor into the first bank of user RAM. */
    void * cp_addr = 0x8000;

    int pd = process_load("COMMAND.EXE");

    if (pd == E_FILENOTFOUND)
    {
        puts("Error: COMMAND.EXE not found.\n\r");
        return;
    }

    if (pd < 0)
    {
        printf("Unknown error loading COMMAND.EXE: %d\n\r", pd);
        return;
    }

    int e = process_spawn(pd, NULL, 0);
#endif

    while (1) {}
}
