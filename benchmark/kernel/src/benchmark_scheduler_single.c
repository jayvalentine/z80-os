#include <stdio.h>

volatile char c;

void main(void)
{
    while (1)
    {
        c = 4;
    }
}
