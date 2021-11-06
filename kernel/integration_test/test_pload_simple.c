#include <syscall.h>
#include <string.h>
#include <stdio.h>

const char file[7] =
{
    0x0a, 0x80, /* header - executable file at base addr 0x8000 */
    0xa5, 0xb6, 0xc7, 0xd8, 0xe9
};

int main()
{
    /* Write file */
    int fd = syscall_fopen("testprog.exe", FMODE_WRITE);
    syscall_fwrite(file, 7, fd);
    syscall_fclose(fd);

    uint16_t address = 0x0000;

    int success = syscall_pload(&address, "testprog.exe");

    if (success != 0) return 1;
    if (address != 0x8000) return 2;

    return 0;
}
