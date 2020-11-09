#include <stddef.h>
#include <syscall.h>
#include <stdio.h>

SIGHANDLER_T sighandler_cancel;

void signal_init()
{
    sighandler_cancel = NULL;
}

void signal_cancel(uint16_t address) __z88dk_fastcall
{
    if (sighandler_cancel != NULL) sighandler_cancel(address);
}