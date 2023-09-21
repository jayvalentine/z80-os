#ifndef _ERRORS_H
#define _ERRORS_H

#include <stdint.h>

typedef uint8_t error_t;

#define ERROR_NOERROR 0
#define ERROR_SYNTAX 1
#define ERROR_UNDEFINED_KW 2
#define ERROR_LINENUM 3
#define ERROR_GOTO 4
#define ERROR_NOT_RUNNING 5
#define ERROR_VARNAME 6
#define ERROR_UNDEFINED_VAR 7
#define ERROR_TOO_MANY_VARS 8
#define ERROR_RETSTACK_EMPTY 9
#define ERROR_RANGE 10
#define ERROR_UNDEFINED_ARRAY 11
#define ERROR_UNKNOWN_FUNC 12

#define ERROR_HANDLE(_err) if (_err != ERROR_NOERROR) return _err
#define ERROR_HANDLE_WITH(_err, _return) if (_err != ERROR_NOERROR) return _return;

void error_display(error_t error);

#endif
