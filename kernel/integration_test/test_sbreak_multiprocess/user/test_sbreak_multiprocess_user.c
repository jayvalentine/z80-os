/* Child process for test_scancel_multiprocess.
 *
 * Expects a single character 'A' on the serial terminal
 * then waits for SIG_CANCEL before exiting.
 */

#include <syscall.h>
#include <string.h>
#include <setjmp.h>
#include <stdint.h>

jmp_buf jmp_env;

int done_cancel;
uint16_t cancel_address;


void cancel_handle(uint16_t address)
{
    done_cancel = 1;
    cancel_address = address;

    longjmp(jmp_env, 1);
}

int user_main(void)
{
    syscall_sighandle(cancel_handle, SIG_CANCEL);
    done_cancel = 0;
    cancel_address = 0;

    int code = setjmp(jmp_env);
    if (code != 0)
    {
        if (done_cancel != 1) return 10;
        
        /* Should have hit the cancel within this program. */
        if (cancel_address < 0xc000 || cancel_address >= 0xd000) return 11;

        return 0;
    }

    while (1)
    {
    }
}