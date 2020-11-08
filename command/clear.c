#include <stddef.h>
#include <stdio.h>

/* Clears the screen and resets the cursor to the origin. */
int command_clear(char ** argv, size_t argc)
{
    /* Clear screen, cursor to top-left. */
    puts("\033[2J\033[1;1H");
    return 0;
}
