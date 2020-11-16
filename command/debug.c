#include <stddef.h>
#include <stdio.h>
#include <syscall.h>

#define INST_BREAK 0xef

uint8_t old_char;

void debug_break(uint16_t address)
{
    printf("BREAK at $%04x\n\r", address);
    *((uint8_t*)address) = old_char;
}

void debug_set_breakpoint(uint8_t * address)
{
    old_char = *address;
    *address = INST_BREAK;
    printf("Breakpoint set at $%04x\n\r", (uint16_t)address);
}

int command_debug(char ** argv, size_t argc)
{
    if (argc < 1)
    {
        puts("No executable provided.\n\r");
        return 1;
    }

    int exitcode;
    char * program = argv[0];

    /* Attempt to open the executable. */
    int program_fd = syscall_fopen(program, FMODE_READ);

    if (program_fd == E_FILENOTFOUND)
    {
        printf("%s could not be found.\n\r", program);
        return 2;
    }
    else
    {
        printf("Loading %s...\n\r", program);
        char * program_ram = 0x8000;
        size_t program_size = 0x7000;
        size_t bytes = syscall_fread(program_ram, program_size, program_fd);

        if (bytes == 0)
        {
            printf("Error reading %s.\n\r", program);
        }
        else
        {
            syscall_sighandle(debug_break, SIG_BREAK);
            debug_set_breakpoint(program_ram);

            /* Execute. */
            exitcode = syscall_pexec(&argv[1], argc-1);
        }
    }

    return exitcode;
}
