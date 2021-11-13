#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

#define TRUE 0x01
#define FALSE 0x00
typedef uint8_t bool;

void set_breakpoint(uint8_t * address);

uint8_t get_nybble_from_char(char c)
{
    if (c < '0') return 0xff;
    if (c <= '9') return (c - '0');
    if (c < 'A') return 0xff;
    if (c <= 'F') return (c - 'A' + 10);
    if (c < 'a') return 0xff;
    if (c <= 'f') return (c - 'a' + 10);

    return 0xff;
}

uint8_t get_byte(const char * str)
{
    uint8_t upper = get_nybble_from_char(str[0]);
    uint8_t lower = get_nybble_from_char(str[1]);
    return (upper << 4) | lower;
}

#define REC_TYPE_DATA 0x00
#define REC_TYPE_EOF 0x01

void process_ihex_record(const char * rec)
{
    uint8_t bytes[255];

    /* Skip over the ':' */
    rec++;

    /* Get number of data bytes. */
    uint8_t data_count = get_byte(rec);
    rec += 2;

    /* Get address. */
    uint16_t address_higher = get_byte(rec);
    rec += 2;
    uint16_t address_lower = get_byte(rec);
    rec += 2;

    uint8_t * address = (uint8_t *)((address_higher << 8) | address_lower);

    /* Now get type of record. */
    uint8_t record_type = get_byte(rec);
    rec += 2;

    /* If type is data, load the bytes at the given address. */
    if (record_type == REC_TYPE_DATA)
    {
        /* Now get bytes. Max is 255. */
        for (uint16_t b = 0; b < data_count; b++)
        {
            uint8_t byte = get_byte(rec);
            rec += 2;
            *address = byte;
            address++;
        }
    }
}

char input[100];

char * cmd;
char ** cmd_argv;
size_t cmd_argc;

bool in_breakpoint;

void parse(char * input)
{
    /* Expect at least one command. */
    cmd = strtok(input, " ");

    if (cmd == NULL) return;

    cmd_argc = 0;
    while (1)
    {
        char * a = strtok(NULL, " ");
        if (a == NULL) break;

        cmd_argv[cmd_argc] = a;
        cmd_argc++;
    }
}

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

uint8_t old_byte;

#define INST_BREAK 0xef
#define BREAKPOINTS_MAX 1
uint8_t breakpoints_set;

void process_commands()
{
    uint16_t last_load_address;

    while (1)
    {
        puts("> ");
        gets(input);

        /* If this is an IHEX record, process it as such. */
        if (input[0] == ':')
        {
            process_ihex_record(input);
            continue;
        }

        /* Otherwise split into arguments. */
        parse(input);

        if (strcmp("run", cmd) == 0)
        {
            if (in_breakpoint)
            {
                puts("Cannot run program while in breakpoint.\r\n");
                continue;
            }

            set_breakpoint((uint8_t *)last_load_address);
            printf("Executing at address $%x...\r\n", last_load_address);
            int ret = syscall_pexec(last_load_address, NULL, 0);
            printf("Program returned $%x.\r\n", ret);
        }
        else if (strcmp("load", cmd) == 0)
        {
            if (in_breakpoint)
            {
                puts("Cannot load program while in breakpoint.\r\n");
                continue;
            }

            printf("Loading %s...\r\n", cmd_argv[0]);
            syscall_pload(&last_load_address, cmd_argv[0]);
            printf("Loaded at address $%x.\r\n", last_load_address);
        }
        else if (strcmp("continue", cmd) == 0)
        {
            if (!in_breakpoint)
            {
                puts("Can only continue when in breakpoint.\r\n");
                continue;
            }

            return;
        }
        else if (strcmp("break", cmd) == 0)
        {
            if (breakpoints_set == BREAKPOINTS_MAX)
            {
                puts("Too many breakpoints set.\r\n");
                continue;
            }

            char * break_address_string = cmd_argv[0];
            uint16_t break_address_upper = get_byte(break_address_string);
            break_address_string += 2;
            uint16_t break_address_lower = get_byte(break_address_string);

            uint16_t break_address = (break_address_upper << 8) | break_address_lower;

            set_breakpoint(break_address);
        }
    }
}

void debug_break(uint16_t arg)
{
    in_breakpoint = TRUE;
    SIGARG_DEBUG_T * d = (SIGARG_DEBUG_T*)arg;
    *((uint8_t*)(d->break_address)) = old_byte;
    breakpoints_set--;

    printf("BREAK at $%04x\n\r\n\r", d->break_address);

    puts("Registers:\n\r");
    printf("A: %02x F: %02x\n\r\n\r", (uint16_t)d->a, (uint16_t)d->f);

    printf("B: %02x C: %02x\n\r", (uint16_t)d->b, (uint16_t)d->c);
    printf("D: %02x E: %02x\n\r", (uint16_t)d->d, (uint16_t)d->e);
    printf("H: %02x L: %02x\n\r\n\r", (uint16_t)d->h, (uint16_t)d->l);

    printf("IX: %04x\n\r", d->ix);
    printf("IY: %04x\n\r\n\r", d->iy);

    process_commands();

    in_breakpoint = FALSE;
}

void set_breakpoint(uint8_t * address)
{
    old_byte = *address;
    *address = INST_BREAK;
    breakpoints_set++;

    printf("Breakpoint set at $%04x\n\r", (uint16_t)address);
}

int user_main(char ** argv, size_t argc)
{
    syscall_sighandle(debug_break, SIG_BREAK);
    
    in_breakpoint = FALSE;
    breakpoints_set = 0;

    puts("Z80 Debugger/Monitor.\n\r");

    process_commands();
    return 0;
}
