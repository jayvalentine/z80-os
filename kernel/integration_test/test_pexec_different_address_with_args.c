#include <syscall.h>
#include <string.h>

int main()
{
    const char * strs[3] =
    {
        "some",
        "arguments",
        "here"
    };

    return syscall_pexec(0xd000, strs, 3);
}
