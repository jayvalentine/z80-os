#include <syscall.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

jmp_buf jmp_env;

int done_break;
uint16_t break_address;

void break_handle(uint16_t address)
{
    done_break = 1;
    break_address = address;

    longjmp(jmp_env, 1);
}

int main(void)
{
    /* Put serial into binary mode. */
    syscall_smode(SMODE_BINARY);

    /* Get a character from serial in. */
    char c = getchar();
    if (c != 0x18) return 3;

    return 0;
}
