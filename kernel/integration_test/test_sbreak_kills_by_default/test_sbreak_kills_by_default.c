#include <syscall.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

jmp_buf jmp_env;

int done_cancel;
uint16_t cancel_address;

#define user_addr (char*)0xc000
#define user_size 2048

/* Program is located at 0xc000 */
const char header[2] = { 0x0a, 0xc0 };

int start_child_process(void)
{
    int fd = syscall_fopen("test.exe", FMODE_WRITE);
    if (fd < 0) return fd;

    size_t bytes = syscall_fwrite(header, 2, fd);
    if (bytes != 2) return 1111;

    size_t bytes2 = syscall_fwrite(user_addr, user_size, fd);
    if (bytes2 != user_size) return 2222;
    
    syscall_fclose(fd);

    int pd = syscall_pload("test.exe");
    if (pd < 0) return pd;

    return syscall_pexec(pd, NULL, 0);
}

void cancel_handle(uint16_t address)
{
    done_cancel = 1;
    cancel_address = address;

    longjmp(jmp_env, 1);
}

int main(void)
{
    syscall_sighandle(cancel_handle, SIG_CANCEL);
    done_cancel = 0;
    cancel_address = 0;

    int child_result = start_child_process();
    if (child_result != -1) return 1;
    
    int code = setjmp(jmp_env);
    if (code != 0)
    {
        if (done_cancel != 1) return 2;

        /* Should have hit the cancel within this program. */
        if (cancel_address < 0x8000 || cancel_address >= 0x9000) return 3;

        return 0;
    }

    char c = getchar();
    if (c != 'C') return 3;

    while (1)
    {
    }
}
