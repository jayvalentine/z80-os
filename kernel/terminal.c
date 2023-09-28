#include <stdint.h>

#include <include/interrupt.h>

uint8_t terminal_current_mode;

void terminal_init(void)
{
    terminal_current_mode = 0;
}

void terminal_mode(uint8_t mode)
{
    terminal_current_mode = mode;
}
