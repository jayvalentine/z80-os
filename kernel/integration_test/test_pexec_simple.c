#include <syscall.h>
#include <string.h>

int main()
{
    syscall_pexec(0x8000, NULL, 0);

    return 0;
}
