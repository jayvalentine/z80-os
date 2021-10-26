#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

unsigned char hex_digit(char digit)
{
    switch (digit)
    {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;

        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;

        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;

        default: return 0xff;
    }
}

/* Convert a hex string to an unsigned integer.
 * str - pointer to string
 * len - number of digits to process
 */
unsigned int hex_to_uint(const char * str, unsigned char len)
{
    unsigned int total = 0;

    for (unsigned char i = 0; i < len; i++)
    {
        total <<= 8;
        total |= hex_digit(*str);

        str++;
    }

    return total;
}

int process_ihex_record(const char * rec)
{
    unsigned char record_length = (unsigned char) hex_to_uint(&rec[1], 2);
    unsigned int address = hex_to_uint(&rec[3], 4);
    unsigned char record_type = (unsigned char) hex_to_uint(&rec[7], 2);

    if (address < 0x8000) return;

    if (record_type == 0x00)
    {
        const char * data = &rec[9];

        for (unsigned char i = 0; i < record_length; i++)
        {
            unsigned char data_byte = (unsigned char) hex_to_uint(data, 2);
            *(unsigned char *)(address + i) = data_byte;
            data += 2;
        }
    }
}

int main(char ** argv, size_t argc)
{
    printf("Z80 Debugger/Monitor.\n\r");
    char command[100];
    while (1)
    {
        /* Get command from user. */
        printf("> ");
        gets(command);

        /* Is it an IHEX record? */
        if (command[0] == ':')
        {
            process_ihex_record(command);
        }
        if (strcmp(command, "run") == 0)
        {
            syscall_pexec(NULL, 0);
        }
    }
}
