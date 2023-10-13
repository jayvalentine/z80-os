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

#define INFO_COL 1
#define INFO_ROW 41

#define SETPOS(_row, _col) printf("\033[%d;%dH", (_row), (_col))
#define SETPOS_CONST_(_row, _col) printf("\033[" #_row ";"  #_col "H")
#define SETPOS_CONST(_row, _col) SETPOS_CONST_(_row, _col)

#define SETPOS_INFO() SETPOS_CONST(INFO_ROW, INFO_COL)

struct
{
    uint8_t row;
    uint8_t col;
} cursor_pos;

void clear_screen(void)
{
    printf("\033[2J\033[1;1H");
}

void update_info_row(uint8_t newpos)
{
    SETPOS_INFO();
    printf("\033[5C%d", (int)newpos);
    SETPOS(newpos, cursor_pos.col);

    cursor_pos.row = newpos;
}

void update_info_col(uint8_t newpos)
{
    SETPOS_INFO();
    printf("\033[14C%d", (int)newpos);
    SETPOS(cursor_pos.row, newpos);

    cursor_pos.col = newpos;
}

void handle_key(char c)
{
    putchar(c);

    /* Keep track of cursor position. */
    if (c == '\r')
    {
        update_info_col(1);
    }
    else if (c == '\n')
    {
        update_info_row(cursor_pos.row + 1);
    }
    else
    {
        update_info_col(cursor_pos.col + 1);
    }
}

int main(size_t argc, char ** argv)
{
    argc; argv;

    /* Initialize cursor position and clear screen. */
    clear_screen();
    update_info_row(1);
    update_info_col(1);

    /* Handle keypresses in a loop. */
    while (1)
    {
        char c = getchar();
        handle_key(c);
    }
}