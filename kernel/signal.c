#include <stddef.h>
#include <syscall.h>
#include <stdio.h>
#include <stdint.h>

#include <include/process.h>
#include <include/bits.h>

SIGHANDLER_T sighandler_cancel;
SIGHANDLER_T sighandler_break;

void signal_init(void)
{
    sighandler_cancel = NULL;
    sighandler_break = NULL;
}

void signal_set(uint8_t signal)
{
    ProcessDescriptor_T * p = process_current();
    BIT_SET(p->sigstatus, signal);
}

void signal_cancel(uint16_t address)
{
    if (sighandler_cancel != NULL) sighandler_cancel(address);
}

void signal_break(uint16_t address)
{
    if (sighandler_break != NULL) sighandler_break(address);
}

void signal_sethandler(SIGHANDLER_T handler, int sig)
{
    if (sig == SIG_CANCEL) sighandler_cancel = handler;
    else if (sig == SIG_BREAK) sighandler_break = handler;
}
