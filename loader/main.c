#include <stdio.h>
#include <string.h>

#include "file.h"
#include "kernel.h"

extern char _tail;

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
        char * mem = &_tail;

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
        load_error("KERNEL.BIN not found");
    }

    return;
}
