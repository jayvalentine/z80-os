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

/* Clears the screen and resets the cursor to the origin. */
int command_clear(char ** argv, size_t argc)
{
    /* Clear screen, cursor to top-left. */
    puts("\033[2J\033[1;1H");
    return 0;
}

/* Get an unsigned integer (in Z80 little-endian)
 * from a buffer at the given position.
 */
uint16_t get_uint16_t(char * buf, size_t i)
{
    uint16_t val;
    uint8_t * val_ptr = (uint8_t *)&val;

    val_ptr[1] = buf[i+1];
    val_ptr[0] = buf[i];
    
    return val;
}

/* Get an unsigned long (in little-endian)
 * from a buffer at a given location.
 */
uint32_t get_uint32_t(char* buf, size_t i)
{
    uint32_t val;
    uint8_t * val_ptr = (uint8_t *)&val;
    
    val_ptr[3] = buf[i+3];
    val_ptr[2] = buf[i+2];
    val_ptr[1] = buf[i+1];
    val_ptr[0] = buf[i];

    return val;
}

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

            if (attr & 0b00000100)  putchar('s');
            else                    putchar('~');

            if (attr & 0b00000010)  putchar('h');
            else                    putchar('~');

            if (attr & 0b00000001)  putchar('r');
            else                    putchar('~');

            puts("  ");

            uint16_t filesize = get_uint16_t(&temp[f], 0x1c);
            printf("%5u", filesize); /* Won't handle files more than 65536 in size. */

            /* Get creation date. */
            uint16_t creation_date = get_uint16_t(&temp[f], 0x10);
            uint16_t year = 1980 + (creation_date >> 9);
            uint16_t month = (creation_date >> 5) & 0x000f;
            uint16_t day = creation_date & 0x001f;

            printf("  %04u-%02u-%02u\n\r", year, month, day);
        }

        sector++;
    }
}

typedef int (*Command_T)(char **, size_t);

#define NUM_COMMANDS 2

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

    puts("Z80-OS Command Processor\n\rCopyright (C) 2020 Jay Valentine\n\r");

    int exitcode = 0;

    while (1)
    {
        puts("> ");

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