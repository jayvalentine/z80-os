#include <syscall.h>
#include <string.h>

int main()
{
    const char * strs[2] =
    {
        "hello",
        "world!"
    };

    return syscall_pexec(0x8000, strs, 2);
}
