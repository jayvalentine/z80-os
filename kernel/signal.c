#include <stddef.h>
#include <syscall.h>
#include <stdio.h>

SIGHANDLER_T sighandler_cancel;
SIGHANDLER_T sighandler_break;

void signal_init()
{
    sighandler_cancel = NULL;
    sighandler_break = NULL;
}

void signal_cancel(uint16_t address) __z88dk_fastcall
{
    if (sighandler_cancel != NULL) sighandler_cancel(address);
}

void signal_break(uint16_t address) __z88dk_fastcall
{
    if (sighandler_break != NULL) sighandler_break(address);
}

void signal_sethandler(SIGHANDLER_T handler, int sig)
{
    if (sig == SIG_CANCEL) sighandler_cancel = handler;
    else if (sig == SIG_BREAK) sighandler_break;
}
