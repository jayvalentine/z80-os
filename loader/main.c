#include <stdio.h>
#include <string.h>

#include "file.h"
#include "kernel.h"

extern char _tail;
extern char reg_state;

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

        puts("Testing low-RAM... ");

        set_reg(0b11111101);
        uint address = ram_test();
        set_reg(0b00000000);

        if (address != 0x8000)
        {
            printf("Error in RAM: %u\n\r", address);
        }
        else
        {
            puts("Done.\n\r");

            /* Copy kernel image into low-RAM and execute. */
            puts("Copying kernel image into low-RAM... ");

            set_reg(0b11111101);
            memcpy((char *)0x0000, &_tail, (uint)file.size);
            set_reg(0b00000000);

            puts("Done.\n\r");
            
            puts("Starting Z80-OS...\n\r");

            /* Wait for a bit (to allow sending of message)
             * before continuing. */
            for (uint i = 0; i < 256; i++) {}

            set_reg(0b11111101);
            kernel();
        }

    }
    else if (error == FILENOTFOUND)
    {
        load_error("KERNEL.BIN not found");
    }

    return;
}
