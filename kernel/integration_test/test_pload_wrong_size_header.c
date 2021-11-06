#include <syscall.h>
#include <string.h>
#include <stdio.h>

const char file[1] =
{
    0x0a
};

int main()
{
    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 1, fd);
    syscall_fclose(fd);

    uint16_t address = 0x1234;

    int success = syscall_pload(&address, "testprog.exe");
    if (address != 0x1234) return 1;
    return success;
}
