#include <syscall.h>

int main(void)
{
    int entries = syscall_fentries();
    return entries;
}
