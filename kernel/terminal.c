#include <include/terminal.h>
#include <include/process.h>
#include <include/bits.h>
#include <include/signal.h>

#define ASCII_CANCEL 0x18

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
inline termstatus_t terminal_status(void)
{
    ProcessDescriptor_T * const p = process_current();
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
    /* If the character is the CANCEL byte and the terminal is
     * in interactive mode, then trigger SIG_CANCEL.
     */
    if (c == ASCII_CANCEL && BIT_IS_CLR(terminal_status(), TERMSTATUS_MODE_BINARY))
    {
        signal_set(SIGSTAT_CANCEL);
    }
    else
    {
        terminal_buf.data[terminal_buf.head++] = c;
    }
}

int terminal_get(void)
{
    if (terminal_buf.head == terminal_buf.tail) return -1;
    
    return (int)terminal_buf.data[terminal_buf.tail++];
}
