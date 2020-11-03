    ; Interface to kernel.
    PUBLIC  _kernel

    ; void kernel(void)
_kernel:
    call    $b000
    ret
