#include <syscall.h>
#include <string.h>
#include <stdio.h>

char file[2050];

int main()
{
    /* Write test file. */
    file[0] = 0x0a;
    file[1] = 0x80;

    int base = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 128; j++)
        {
            file[base+j+2] = (char)(i+10);
        }
        base += 128;
    }

    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 2050, fd);
    syscall_fclose(fd);

    uint16_t address = 0x0000;

    int success = syscall_pload(&address, "testprog.exe");

    if (success != 0) return 1;
    if (address != 0x8000) return 2;

    return 0;
}
