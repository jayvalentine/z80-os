#include <syscall.h>
#include <string.h>
#include <stdio.h>

const char file[7] =
{
    0x0b, 0x80, /* invalid header - executable file at base addr 0x8000 */
    0xa5, 0xb6, 0xc7, 0xd8, 0xe9
};

int main(void)
{
    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 7, fd);
    syscall_fclose(fd);

    int success = syscall_pload("testprog.exe");
    return success;
}
