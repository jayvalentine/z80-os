#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <syscall.h>
#include <stdint.h>

typedef uint8_t sigstatus_t;

#define SIGSTAT_CANCEL (1 << 0)
#define SIGSTAT_BREAK  (1 << 1)

void signal_init(void);

/* signal_set
 *
 * Sets the signal flag for the given signal
 * for the current process.
 */
void signal_set(uint8_t);

#endif /* SIGNAL_H */
