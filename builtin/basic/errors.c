#include "errors.h"

#include <stdio.h>

void error_display(error_t error)
{
    if (error == ERROR_SYNTAX) puts("SYNTAX!\r\n");
}