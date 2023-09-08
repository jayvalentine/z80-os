#include <syscall.h>
#include <string.h>
#include <stdio.h>

char file[2050];
int base;

int main(void)
{
    /* Write test file. */
    file[0] = 0x0a;
    file[1] = 0x80;

    base = 0;
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

    int pd = syscall_pload("testprog.exe");

    if (pd != 1) return 1;

    return 0;
}
