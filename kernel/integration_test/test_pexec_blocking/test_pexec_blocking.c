#include <syscall.h>
#include <string.h>

#define user_addr (char*)0xc000
#define user_size 2048

const char header[2] = { 0x0a, 0x80 };

int main(void)
{
    int fd = syscall_fopen("test.exe", FMODE_WRITE);
    if (fd < 0) return fd;

    size_t bytes = syscall_fwrite(header, 2, fd);
    if (bytes != 2) return 123;

    size_t bytes2 = syscall_fwrite(user_addr, user_size, fd);
    if (bytes2 != user_size) return 124;
    
    syscall_fclose(fd);

    int pd = syscall_pload("test.exe");
    if (pd < 0) return pd;
    
    return syscall_pexec(pd, NULL, 0);
}
