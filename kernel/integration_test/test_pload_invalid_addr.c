#include <syscall.h>
#include <string.h>
#include <stdio.h>

const char file[7] =
{
    0x0a, 0x50, /* header - executable file at base addr 0x5000 - invalid! */
    0x12, 0x34, 0x56, 0x78, 0x9a
};

int main()
{
    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 7, fd);
    syscall_fclose(fd);

    int success = syscall_pload("testprog.exe");
    return success;
}
