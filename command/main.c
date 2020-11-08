/* Z80-OS Command Processor.
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
#include <string.h>
#include <stddef.h>
#include <syscall.h>

#include "clear.h"
#include "chmod.h"
#include "dir.h"

char input[256];
char * cmd;
char * argv[16];
size_t argc;
char program[16];

char temp[512];

void parse(char * input)
{
    /* Expect at least one command. */
    cmd = strtok(input, " ");

    if (cmd == NULL) return;

    argc = 0;
    while (1)
    {
        char * a = strtok(NULL, " ");
        if (a == NULL) break;

        argv[argc] = a;
        argc++;
    }
}

/* In-place "upper-cases" the given string. */
void toupper(char * s)
{
    while (*s != '\0')
    {
        if (*s >= 'a' && *s <= 'z')
        {
            *s -= ('a' - 'A');
        }
        s++;
    }
}

typedef int (*Command_T)(char **, size_t);

#define NUM_COMMANDS 3

#define program_main ((Command_T)0x8000)

typedef struct _Inbuilt
{
    char * name;
    Command_T run;
} Inbuilt_T;

const Inbuilt_T commands[NUM_COMMANDS] =
{
    {
        "CLEAR",
        &command_clear
    },
    {
        "DIR",
        &command_dir
    },
    {
        "CHMOD",
        &command_chmod
    }
};

Inbuilt_T * get_command(const char * s)
{
    for (size_t c = 0; c < NUM_COMMANDS; c++)
    {
        Inbuilt_T * command = &commands[c];
        if (strcmp(cmd, command->name) == 0)
        {
            return command;
        }
    }

    return NULL;
}

void main()
{
    /* Clear screen, cursor to top-left. */
    puts("\033[2J\033[1;1H");

    /* Green text on black background. */
    puts("\033[32;40m");

    puts("Z80-OS Command Processor\n\rCopyright (C) 2020 Jay Valentine\n\r\n\r");

    int exitcode = 0;

    while (1)
    {
        printf("(%u) > ", exitcode);

        /* Get user input and parse into cmd and argv */
        gets(input);
        parse(input);

        toupper(cmd);

        Inbuilt_T * command_to_run = get_command(cmd);
        
        if (command_to_run == NULL)
        {
            size_t cmd_len = strlen(cmd);
            if (cmd_len > 8) cmd_len = 8;

            /* Construct name of file to load. */
            memcpy(program, cmd, cmd_len);
            program[cmd_len] = '.';
            memcpy(&program[cmd_len+1], "EXE", 4);

            int program_fd = syscall_fopen(program, FMODE_READ);

            if (program_fd == E_FILENOTFOUND)
            {
                printf("%s could not be found\n\r", program);
            }
            else
            {
                printf("Loading %s...\n\r", program);
                char * program_ram = 0x8000;
                size_t program_size = 0x7000;
                size_t bytes = syscall_fread(program_ram, program_size, program_fd);

                if (bytes == 0)
                {
                    printf("Error reading %s\n\r", program);
                }
                else
                {
                    exitcode = program_main(argv, argc);
                }
            }
        }
        else
        {
            exitcode = command_to_run->run(argv, argc);
        }
    }
}