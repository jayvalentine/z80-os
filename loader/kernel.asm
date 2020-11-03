    ; Interface to kernel.
    PUBLIC  _kernel

    EXTERN  __tail

    ; void kernel(void)
_kernel:
    call    __tail
    ret
