#include <syscall.h>
#include <string.h>

int main(char ** argv, size_t argc)
{
    syscall_pexec(NULL, 0);
}
