#include "file.h"
#include "kernel.h"
#include "utils.h"

extern char _tail;
extern char reg_state;

File_T file;
int error;

void main(void)
{
    puts("Reading filesystem... ");

    error = filesystem_init();
    if (error != 0)
    {
        puts("Error reading filesystem");
        return;
    }

    puts("Done.\n\r");

    puts("Loading kernel... ");
    error = file_open("KERNEL.BIN", &file);

    if (error == 0)
    {
        /* Read the file into memory.
         * We have temporary memory in the first 512 bytes after tail,
         * so we need to make sure not to overwrite that when loading. */
        char * mem = &_tail + 512;

        /* Only read up to 0xf000, to avoid trashing system variables. */
        file_read((ubyte *)mem, &file, (size_t)(0xf000 - (size_t)mem));

        puts("Done.\n\r");

        puts("Testing low-RAM... ");

        set_reg(0b11111101);
        uint address = ram_test();
        set_reg(0b00000000);

        if (address != 0x8000)
        {
            puts("RAM error.\n\r");
        }
        else
        {
            puts("Done.\n\r");

            /* Copy kernel image into low-RAM and execute. */
            puts("Copying kernel image... ");

            /* Wait for a bit (to allow sending of message)
             * before continuing. */
            for (uint i = 0; i < 10; i++) {}

            set_reg(0b11111101);
            memcpy((char *)0x0000, mem, (uint)file.size);
            set_reg(0b00000000);

            puts("Done.\n\r");
            
            puts("Starting Z80-OS...\n\r");

            /* Wait for a bit (to allow sending of message)
             * before continuing. */
            for (uint i = 0; i < 10; i++) {}

            set_reg(0b11111111);
            kernel();
        }

    }
    else if (error == E_FILENOTFOUND)
    {
        puts("KERNEL.BIN not found\n\r");
    }

    return;
}
