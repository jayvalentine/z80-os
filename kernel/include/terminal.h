#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <stdint.h>

/* Type used for representing a process's terminal status. */
typedef uint8_t termstatus_t;

/* 0 = interactive, 1 = binary */
#define TERMSTATUS_MODE_BINARY (1 << 0)

/* 0 = inactive, 1 = active */
#define TERMSTATUS_ACTIVE (1 << 1)

/* 0 = no data, 1 = data ready */
#define TERMSTATUS_AVAILABLE (1 << 2)

#endif
