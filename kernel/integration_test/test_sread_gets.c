#include <syscall.h>
#include <string.h>
#include <stdio.h>

char buf[10];

int main()
{
    gets(buf);

    if (buf[0] != 'h') return 1;
    if (buf[1] != 'e') return 2;
    if (buf[2] != 'l') return 3;
    if (buf[3] != 'l') return 4;
    if (buf[4] != 'o') return 5;
    if (buf[5] != '\0') return 6;

    return 0;
}
