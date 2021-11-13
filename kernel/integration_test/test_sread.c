#include <syscall.h>
#include <string.h>
#include <stdio.h>

int main()
{
    char c;
    
    c = getchar();
    if (c != 'h') return 1;

    c = getchar();
    if (c != 'e') return 2;

    c = getchar();
    if (c != 'l') return 3;

    c = getchar();
    if (c != 'l') return 4;

    c = getchar();
    if (c != 'o') return 5;

    return 0;
}
