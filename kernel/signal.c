#include <stddef.h>
#include <syscall.h>
#include <stdio.h>

SIGHANDLER_T sighandler_cancel;
SIGHANDLER_T sighandler_break;

void signal_init(void)
{
    sighandler_cancel = NULL;
    sighandler_break = NULL;
}

#ifdef Z88DK
void signal_cancel(uint16_t address) __z88dk_fastcall
#else
void signal_cancel(uint16_t address)
#endif
{
    if (sighandler_cancel != NULL) sighandler_cancel(address);
}

#ifdef Z88DK
void signal_break(uint16_t address) __z88dk_fastcall
#else
void signal_break(uint16_t address)
#endif
{
    if (sighandler_break != NULL) sighandler_break(address);
}

void signal_sethandler(SIGHANDLER_T handler, int sig)
{
    if (sig == SIG_CANCEL) sighandler_cancel = handler;
    else if (sig == SIG_BREAK) sighandler_break = handler;
}
