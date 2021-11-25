#include "errors.h"

#include <stdio.h>

#include "program.h"

void error_display(error_t error)
{
    if (error == ERROR_SYNTAX) puts("SYNTAX!");
    else if (error == ERROR_UNDEFINED_KW) puts ("KEYWORD?");
    else if (error == ERROR_LINENUM) puts("LINENUM?");
    else if (error == ERROR_GOTO) printf("GOTO %u?", program_next_lineno());
    else if (error == ERROR_NOT_RUNNING) puts("PROGRAM NOT RUNNING!");
    else if (error == ERROR_RANGE) puts("OUT OF RANGE!");
    else puts("UNDEFINED ERROR!");

    puts("\r\n");
}
