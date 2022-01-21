#include <syscall.h>
#include <string.h>

#define user_addr (char*)0xc000
#define user_size 2048

const char header[2] = { 0x0a, 0x80 };

int main()
{
    const char * strs[2] =
    {
        "hello",
        "world!"
    };
    
    int fd = syscall_fopen("test.exe", FMODE_WRITE);
    syscall_fwrite(header, 2, fd);
    syscall_fwrite(user_addr, user_size, fd);
    syscall_fclose(fd);

    int pd = syscall_pload("test.exe");
    return syscall_pexec(pd, strs, 2);
}
