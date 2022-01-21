#include <syscall.h>
#include <string.h>

#define user_addr (char*)0xc000
#define user_size 2048

const char header[2] = { 0x0a, 0xd0 };

int main()
{
    const char * strs[3] =
    {
        "some",
        "arguments",
        "here"
    };

    int f = syscall_fopen("file.exe", FMODE_WRITE);
    syscall_fwrite(header, 2, f);
    syscall_fwrite(user_addr, user_size, f);
    syscall_fclose(f);

    int p = syscall_pload("file.exe");
    return syscall_pexec(p, strs, 3);
}
