#include <syscall.h>
#include <string.h>

const char file[3] = { 0x0a, 0xd0, 0x00 };

int main()
{
    int f = syscall_fopen("file.exe", FMODE_WRITE);
    syscall_fwrite(file, 3, f);
    syscall_fclose(f);

    int p = syscall_pload("file.exe");
    syscall_pexec(p, NULL, 0);

    return 0;
}
