#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* If we're compiling for Z80
 * (i.e. we're compiling to run on the desktop for testing)
 * then we need to make sure the code will compile.
 */
#ifdef Z80
#include <file.h>

FILE thefile;

/* NASTY!!! */
#define main(_argc, _argv) user_main(_argv, _argc)
#define ARG_LIM 1
#define ARG_FIRST 0

#else
#define ARG_LIM 2
#define ARG_FIRST 1
#endif

struct
{
    uint8_t x;
    uint8_t y;
} cursor_pos;

void clear_screen(void)
{
    printf("\033[2J\033[1;1H");
}

void handle_key(char c)
{
    putchar(c);

    /* Keep track of cursor position. */
    if (c == '\r') cursor_pos.x = 1;
    else if (c == '\n') cursor_pos.y++;
    else cursor_pos.x++;
}

int main(size_t argc, char ** argv)
{
    argc; argv;

    /* Initialize cursor position and clear screen. */
    cursor_pos.x = 1;
    cursor_pos.y = 1;
    clear_screen();

    /* Handle keypresses in a loop. */
    while (1)
    {
        char c = getchar();
        handle_key(c);
    }
}