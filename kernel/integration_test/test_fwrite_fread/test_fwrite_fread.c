#include <syscall.h>
#include <string.h>

#define FILE_SIZE 22
const char file_contents[FILE_SIZE] =
{
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    100, 102, 104, 106, 108, 110, 112, 114, 116, 118,
    42, 43
};

int main(void)
{
    int fd;

    fd = syscall_fopen("myfile.txt", FMODE_WRITE);
    syscall_fwrite(file_contents, FILE_SIZE, fd);
    syscall_fclose(fd);

    fd = syscall_fopen("myfile.txt", FMODE_READ);
    char read_contents[FILE_SIZE];
    size_t bytes = syscall_fread(read_contents, FILE_SIZE, fd);
    syscall_fclose(fd);

    /* Check number of bytes read is correct. */
    if (bytes != FILE_SIZE) return 1;

    /* Check contents are correct. */
    for (int i = 0; i < FILE_SIZE; i++)
    {
        if (file_contents[i] != read_contents[i]) return 100 + i;
    }
    
    /* Test passed. */
    return 0;
}
