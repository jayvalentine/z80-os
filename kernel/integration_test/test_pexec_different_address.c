#include <syscall.h>
#include <string.h>

int main()
{
    syscall_pexec(0xd000, NULL, 0);

    return 0;
}
