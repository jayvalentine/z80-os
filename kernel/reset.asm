    ; Reset vectors for Z80-OS kernel.

    EXTERN  _main

_reset:
    ld      A, 0b01010101
    out     ($80), A
    call    _main
    halt
    
    defs    $1234
