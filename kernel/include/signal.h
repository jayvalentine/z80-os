#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <syscall.h>
#include <stdint.h>

typedef uint8_t sigstatus_t;

#define SIGSTAT_CANCEL (1 << 0)
#define SIGSTAT_BREAK  (1 << 1)

/* Structure to hold address of handler functions
 * for each signal.
 */
typedef struct _sighandlers_t
{
    SIGHANDLER_T cancel;
    SIGHANDLER_T brk;
} sighandlers_t;

/* signal_set
 *
 * Sets the signal flag for the given signal
 * for the current process.
 */
void signal_set(uint8_t);

/* signal_get_handler
 *
 * Gets the address of the handler for the active signal
 * for the current process, or 0 if no signal is active.
 */
SIGHANDLER_T signal_get_handler(void);

#endif /* SIGNAL_H */
