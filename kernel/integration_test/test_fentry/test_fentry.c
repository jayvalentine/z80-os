#include <syscall.h>
#include <string.h>

int main()
{
    int fd;
    int e;
    char filename[20];

    fd = syscall_fopen("afile.txt", FMODE_WRITE);
    syscall_fclose(fd);

    fd = syscall_fopen("file2.log", FMODE_WRITE);
    syscall_fclose(fd);

    fd = syscall_fopen("nextfile.asm", FMODE_WRITE);
    syscall_fclose(fd);
    
    /* Check names of files returned by fentry syscall. */
    e = syscall_fentry(filename, 0);
    if (e != 0) return 1;
    if (strcmp(filename, "AFILE.TXT") != 0) return 2;

    e = syscall_fentry(filename, 1);
    if (e != 0) return 3;
    if (strcmp(filename, "FILE2.LOG") != 0) return 4;

    e = syscall_fentry(filename, 2);
    if (e != 0) return 5;
    if (strcmp(filename, "NEXTFILE.ASM") != 0) return 6;
    
    /* Should return an error
     * because there are only three files on disk. */
    e = syscall_fentry(filename, 3);
    if (e != E_FILENOTFOUND) return 7;
    
    /* Test passed. */
    return 0;
}
