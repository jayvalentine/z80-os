#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <stdint.h>

#include "errors.h"

uint16_t program_free(void);

void program_new(void);

error_t program_insert(const uint8_t * toks);

void program_list(void);

error_t program_run(void);

#endif
