#include <stddef.h>
#include <syscall.h>
#include <stdio.h>
#include <stdint.h>

#include <include/process.h>
#include <include/bits.h>

void signal_set(uint8_t signal)
{
    ProcessDescriptor_T * p = process_current();
    BIT_SET(p->sigstatus, signal);
}

void signal_sethandler(SIGHANDLER_T handler, int sig)
{
    ProcessDescriptor_T * p = process_current();

    if (sig == SIG_CANCEL) p->sighandlers.cancel = handler;
    else if (sig == SIG_BREAK) p->sighandlers.brk = handler;
}

SIGHANDLER_T signal_get_handler(void)
{
    ProcessDescriptor_T * p = process_current();

    /* Return 0 if no signal set. */
    if (p->sigstatus == 0) return 0;

    if (BIT_IS_SET(p->sigstatus, SIGSTAT_CANCEL))
    {
        /* Reset signal. */
        BIT_CLR(p->sigstatus, SIGSTAT_CANCEL);

        /* Return signal handler for current process. */
        return p->sighandlers.cancel;
    }

    return 0;
}
