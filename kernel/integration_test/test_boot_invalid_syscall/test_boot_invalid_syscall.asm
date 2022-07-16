    .globl  _main

_main:
    ; Invalid syscall - odd offset.
    ld      A, #5
    rst     48
    ret
    
    ; Linker fails without this data definition
    .area   _DATA (REL)
_data:
    .ds     1
