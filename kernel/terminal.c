#include <stdint.h>
#include <stdbool.h>

struct _TerminalBuf
{
    char data[256];
    uint8_t head;
    uint8_t tail;
} terminal_buf;

void terminal_init(void)
{
    terminal_buf.head = 0;
    terminal_buf.tail = 0;
}

void terminal_put(char c)
{
    // TODO: Handle SIG_CANCEL here!
    
    terminal_buf.data[terminal_buf.head++] = c;
}

char terminal_get(void)
{
    return terminal_buf.data[terminal_buf.tail++];
}

bool terminal_available(void)
{
    return terminal_buf.head != terminal_buf.tail;
}
