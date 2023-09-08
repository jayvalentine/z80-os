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
    syscall_sighandle(break_handle, SIG_CANCEL);
    
    int code = setjmp(jmp_env);
    if (code != 0)
    {
        if (done_break != 1) return 1;
        if (break_address < 0x6000) return 2;

        return 0;
    }

    while (1)
    {
    }
}
