/* Z80-OS Kernel. */

#include <stdio.h>

char input[256];

void main(void)
{
    puts("This is the Z80-OS kernel!\n\r");

    while (1)
    {
        puts("> ");
        gets(input);
        printf("You typed: %s\n\r", input);
    }
}
