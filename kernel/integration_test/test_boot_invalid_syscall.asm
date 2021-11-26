    PUBLIC  _main

_main:
    ; Invalid syscall - odd offset.
    ld      A, 5
    rst     48
    ret
