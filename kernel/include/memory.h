#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

/* Error codes. */
#define E_NOPAGES -1
#define E_INIT -2

/* Page disabled, e.g. because of lack of memory. */
#define PAGE_DISABLED 0x00

/* Page free to be allocated if needed. */
#define PAGE_FREE 0x01

/* Page in use. */
#define PAGE_USED 0x02

int memory_init(int pages);
int memory_allocate(void);
void memory_free(int page);

#endif
