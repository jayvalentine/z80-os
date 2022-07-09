#include <syscall.h>

int main()
{
    int fd;
    fd = syscall_fopen("file1.txt", FMODE_WRITE);
    syscall_fclose(fd);

    fd = syscall_fopen("file2.txt", FMODE_WRITE);
    syscall_fclose(fd);

    fd = syscall_fopen("file3.txt", FMODE_WRITE);
    syscall_fclose(fd);
    
    int entries = syscall_fentries();
    return entries;
}
