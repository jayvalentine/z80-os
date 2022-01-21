#include <syscall.h>
#include <string.h>

const char other_proc[4] = {
    0x0a, 0x80,
    0x18, 0xfe          /* infinite loop */
};

void loop(void);

void main()
{
    int fd = syscall_fopen("test.exe", FMODE_WRITE);
    syscall_fwrite(other_proc, 4, fd);
    syscall_fclose(fd);

    int pd = syscall_pload("test.exe");
    int e = syscall_pspawn(pd, NULL, 0);

    loop();
}
