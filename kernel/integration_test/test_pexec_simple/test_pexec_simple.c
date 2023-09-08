#include <syscall.h>
#include <string.h>

const char file[3] = { 0x0a, 0x80, 0x00 };

int main(void)
{
    int f = syscall_fopen("file.exe", FMODE_WRITE);
    syscall_fwrite(file, 3, f);
    syscall_fclose(f);

    int p = syscall_pload("file.exe");
    if (p != 1) return p;
    
    syscall_pexec(p, NULL, 0);

    return 0;
}
