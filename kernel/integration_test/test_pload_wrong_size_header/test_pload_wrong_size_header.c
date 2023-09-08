#include <syscall.h>
#include <string.h>
#include <stdio.h>

const char file[1] =
{
    0x0a
};

int main(void)
{
    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 1, fd);
    syscall_fclose(fd);

    int success = syscall_pload("testprog.exe");
    return success;
}
