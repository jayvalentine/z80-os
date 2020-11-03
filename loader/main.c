#include <stdio.h>
#include <string.h>

#include "file.h"
#include "kernel.h"

char input[256];
char * argv[256];

size_t argc;
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
    error = file_open("KERNEL.BIN", &file);

    if (error == NOERROR)
    {
        /* Read the file into memory. */
        char * mem = 0xb000;

        while (TRUE)
        {
            int c = file_readbyte(&file);
            if (c == EOF) break;

            *mem = (ubyte)c;
            mem++;
        }

        puts("Done.\n\r");

        /* Enter kernel. */
        kernel();
    }
    else if (error == FILENOTFOUND)
    {
        load_error("KERNEL.HEX not found");
    }

    return;
}
