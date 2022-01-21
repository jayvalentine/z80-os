#ifndef _RAM_H
#define _RAM_H

#include <stdint.h>

#ifdef UNIT_TEST
void ram_bank_set(uint8_t bank);
#else
void ram_bank_set(uint8_t bank) __z88dk_fastcall;
#endif

uint16_t ram_bank_test(void);
uint8_t ram_bank_current(void);

/* ram_copy
 *
 * Purpose:
 *     Copies data from memory in one bank to memory
 *     in another. After execution, selected bank
 *     will be the same as on entry.
 * 
 * Parameters:
 *     dst:  Destination pointer
 *     bank: Destination RAM bank
 *     src:  Source pointer
 *     n:    Size of data to copy
 * 
 * Returns:
 *     Nothing.
 */
void ram_copy(char * dst, uint8_t bank, char * src, size_t n);

#endif
