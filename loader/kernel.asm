    ; Interface to kernel.
    PUBLIC  _kernel
    PUBLIC  _set_reg

    EXTERN  __tail

    ; void kernel(void)
_kernel:
    call    __tail
    ret

    ; void set_reg(ubyte val) __z88dk_fastcall
_set_reg:
    ld      A, L
    out     ($80), A
    ret
