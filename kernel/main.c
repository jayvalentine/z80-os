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
#include <include/disk.h>
#include <include/ram.h>
#include <include/process.h>
#include <include/memory.h>
#include <include/scheduler.h>

#include <include/driver_8254.h>

extern SysInfo_T sysinfo;

char input[256];

uint8_t startup_flags;

typedef void (*proc_t)(void);

void interrupt_disable(void);
void interrupt_enable(void);
void debug_process_run(void);

extern const char kernel_version;

void main(void)
{
    interrupt_disable();

    status_init();
    status_set_kernel();

    disk_init();
    filesystem_init();

    /* Get number of RAM banks. */
    uint8_t banks = ram_bank_test();
    sysinfo.numbanks = banks;
    ram_bank_set(banks-1);

    int e = memory_init(banks);
    if (e != 0)
    {
        printf("Memory initialization error: %d\r\n", e);
    }

    process_init();

    scheduler_init();

    terminal_init();

#ifndef DEBUG
    //printf("Z80-OS KERNEL v%s\r\n", &kernel_version);
    //printf("Memory: %d banks\r\n", (int)sysinfo.numbanks);

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
        //puts("Done.\n\r");
    }

    //puts("Loading command processor... ");

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

    int e2 = process_spawn(pd, NULL, 0);
#endif

    timer_init();

    status_clr_kernel();
    interrupt_enable();

    while (1) {}
}
