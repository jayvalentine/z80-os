    EXTERN  _main

_reset:
    pop     HL
    ld      HL, _done
    push    HL

    jp      _main

_done:
    halt

    PUBLIC  _loop

_loop:
    jp      _loop
