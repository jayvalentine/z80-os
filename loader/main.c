#include <stdio.h>
#include <string.h>

#include "file.h"

char input[256];

size_t argc;
char * argv[256];
char * cmd;

void load_error(const char * msg)
{
    printf("Error loading kernel: %s\n\r", msg);
}

void main(void)
{
    File_T file;
    FileError_T error;

    puts("Z80-OS\n\rCopyright (C) 2020 Jay Valentine.\n\r");
    
    puts("Reading filesystem... ");

    error = filesystem_init();
    if (error != NOERROR)
    {
        load_error("error reading filesystem");
        return;
    }

    puts("Done.\n\r");

    puts("Loading kernel... ");
    error = file_open("KERNEL.HEX", &file);

    if (error == NOERROR)
    {
        
    }
    else if (error == FILENOTFOUND)
    {
        load_error("KERNEL.HEX not found");
    }

    return;
}
