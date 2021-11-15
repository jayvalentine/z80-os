#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <stdint.h>

#include "errors.h"

uint16_t program_free(void);

void program_new(void);

error_t program_insert(const uint8_t * toks);

void program_list(void);

error_t program_run(void);

/* program_current_lineno
 *
 * Purpose:
 *     Get the current line number of the executing program.
 *
 * Parameters:
 *     Nothing.
 *
 * Returns:
 *     Integer line number.
 */
int program_current_lineno(void);

/* program_next_lineno
 *
 * Purpose:
 *     Get the next line number to be executed.
 *     Mainly used for error reporting.
 * 
 * Parameters:
 *     Nothing.
 * 
 * Returns:
 *     Integer line number.
 */
int program_next_lineno(void);

#endif
