#include <syscall.h>
#include <string.h>
#include <stdio.h>

const char file[7] =
{
    0x0a, 0xd0, /* header - executable file at base addr 0xd000 */
    0x11, 0x22, 0x33, 0x44, 0x55
};

int main()
{
    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 7, fd);
    syscall_fclose(fd);

    int success = syscall_pload("testprog.exe");

    if (success != 0) return 1;

    return 0;
}
