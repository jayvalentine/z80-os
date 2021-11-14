#ifndef _ERRORS_H
#define _ERRORS_H

#include <stdint.h>

typedef uint8_t error_t;

#define ERROR_NOERROR 0
#define ERROR_SYNTAX 1

void error_display(error_t error);

#endif
