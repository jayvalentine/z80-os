#include <syscall.h>
#include <string.h>

#define user_addr (char*)0xc000
#define user_size 2048

const char header[2] = { 0x0a, 0xd0 };

int main(void)
{
    const char * strs[3] =
    {
        "some",
        "arguments",
        "here"
    };

    int f = syscall_fopen("file.exe", FMODE_WRITE);
    if (f < 0) return f;

    size_t bytes = syscall_fwrite(header, 2, f);
    if (bytes != 2) return 123;

    size_t bytes2 = syscall_fwrite(user_addr, user_size, f);
    if (bytes2 != user_size) return 124;

    syscall_fclose(f);

    int p = syscall_pload("file.exe");
    if (p < 0) return p;

    return syscall_pexec(p, strs, 3);
}
