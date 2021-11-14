#include "errors.h"

#include <stdio.h>

void error_display(error_t error)
{
    if (error == ERROR_SYNTAX) puts("SYNTAX!\r\n");
    else if (error == ERROR_UNDEFINED_KW) puts ("KEYWORD?\r\n");
    else if (error == ERROR_LINENUM) puts("LINENUM?\r\n");
}