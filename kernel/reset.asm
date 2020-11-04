    ; Reset vectors for Z80-OS kernel.

    EXTERN  _start
    EXTERN  _syscall_handler
    EXTERN  _interrupt_handler
    
    ; Entry point.
_reset:
    ; Disable interrupts on startup.
    di
    jp      _start

    defs    0x0030 - ASMPC
_syscall_entry:
    jp      _syscall_handler

    defs    0x0038 - ASMPC
_interrupt_entry:
    jp      _interrupt_handler
