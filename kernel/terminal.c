#include <include/terminal.h>
#include <include/process.h>
#include <include/bits.h>

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

/* Gets terminal status of the current process. */
termstatus_t terminal_status(void)
{
    ProcessDescriptor_T * p = process_current();
    return p->termstatus;
}

/* Sets terminal mode of the current process. */
void terminal_set_mode(int mode)
{
    ProcessDescriptor_T * p = process_current();

    if (mode)
    {
        BIT_SET(p->termstatus, TERMSTATUS_MODE_BINARY);
    }
    else
    {
        BIT_CLR(p->termstatus, TERMSTATUS_MODE_BINARY);
    }
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
