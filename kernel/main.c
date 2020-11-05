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

char input[256];

void main(void)
{
    puts("Initialising kernel... ");

    puts("Done.\n\r");

    puts("Loading command processor... ");

    /* Load command processor into the last 8k of low-RAM. */
    void * cp_addr = 0x6000;
    int fd = syscall_fopen("COMMAND.BIN", FMODE_READ);
    
    /* size_t bytes = syscall_fread(cp_addr, 0x2000, fd); */

    puts("Done.\n\r");
}
