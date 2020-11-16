#include <stddef.h>
#include <stdio.h>
#include <syscall.h>
#include <string.h>

#include "utils.h"

#define INST_BREAK 0xef

uint8_t old_char;

typedef struct _SIGARG_DEBUG
{
    uint16_t break_address;

    uint8_t f;
    uint8_t a;

    uint8_t c;
    uint8_t b;

    uint8_t e;
    uint8_t d;

    uint8_t l;
    uint8_t h;

    uint16_t ix;
    uint16_t iy;

} SIGARG_DEBUG_T;

char * debug_cmd;
char * debug_arg;

void debug_parse(char * str)
{
    /* Expect at least one command. */
    debug_cmd = strtok(str, " ");

    if (debug_cmd == NULL) return;

    debug_arg = strtok(NULL, " ");
}

void debug_set_breakpoint(uint8_t * address)
{
    old_char = *address;
    *address = INST_BREAK;
    printf("Breakpoint set at $%04x\n\r", (uint16_t)address);
}

void debug_command(void)
{
    char input[32];
    while (1)
    {
        puts("DEBUG> ");
        
        /* Get line of input from user. */
        gets(input);
        debug_parse(input);

        utils_toupper(debug_cmd);
        
        if (strcmp(debug_cmd, "BREAK") == 0)
        {
            uint16_t address = utils_strtou(debug_arg);
            debug_set_breakpoint(address);
        }
        else if (strcmp(debug_cmd, "RUN") == 0)
        {
            return;
        }
    }
}

void debug_break(uint16_t arg)
{
    SIGARG_DEBUG_T * d = (SIGARG_DEBUG_T*)arg;

    printf("BREAK at $%04x\n\r\n\r", d->break_address);
    
    puts("Registers:\n\r");
    printf("A: %02x F: %02x\n\r\n\r", (uint16_t)d->a, (uint16_t)d->f);

    printf("B: %02x C: %02x\n\r", (uint16_t)d->b, (uint16_t)d->c);
    printf("D: %02x E: %02x\n\r", (uint16_t)d->d, (uint16_t)d->e);
    printf("H: %02x L: %02x\n\r\n\r", (uint16_t)d->h, (uint16_t)d->l);

    printf("IX: %04x\n\r", d->ix);
    printf("IY: %04x\n\r\n\r", d->iy);

    *((uint8_t*)(d->break_address)) = old_char;

    debug_command();
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
