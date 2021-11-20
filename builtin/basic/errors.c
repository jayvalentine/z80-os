#include "errors.h"

#include <stdio.h>

#include "program.h"

void error_display(error_t error)
{
    if (error == ERROR_SYNTAX) puts("SYNTAX!\r\n");
    else if (error == ERROR_UNDEFINED_KW) puts ("KEYWORD?\r\n");
    else if (error == ERROR_LINENUM) puts("LINENUM?\r\n");
    else if (error == ERROR_GOTO) printf("GOTO %u?\r\n", program_next_lineno());
    else if (error == ERROR_NOT_RUNNING) puts("PROGRAM NOT RUNNING!\r\n");
    else puts("UNDEFINED ERROR!\r\n");
}
