#include <syscall.h>

int main()
{
    int entries = syscall_fentries();
    return entries;
}
