    PUBLIC  _break_handler

    EXTERN  _signal_break

_break_handler:
    ; Save all registers onto stack.
    push    AF
    
    push    BC
    push    DE
    push    HL

    push    IX
    push    IY

    ; Put register values into struct.
    ld      (_debug_args_bc), BC
    ld      (_debug_args_de), DE
    ld      (_debug_args_hl), HL
    ld      (_debug_args_ix), IX
    ld      (_debug_args_iy), IY

    ; Special handling for AF seeing as we can't directly load that into memory.
    push    AF
    pop     HL
    ld      (_debug_args_af), HL

    ; Get break address into HL and store in struct.
    ld      IX, 12
    add     IX, SP
    ld      L, (IX+0)
    ld      H, (IX+1)

    dec     HL ; Decrement return address to get address of rst instruction.
    ld      (_debug_args_address), HL

    ; Save break address and location on stack across function call.
    push    IX
    push    HL

    ; Pass struct to break signal handler.
    ld      HL, _debug_args
    call    _signal_break

    ; Set return address to address of BREAK instruction.
    pop     HL
    pop     IX
    ld      (IX+0), L
    ld      (IX+1), H

    ; Pop registers off stack.
    pop     IY
    pop     IX

    pop     HL
    pop     DE
    pop     BC

    pop     AF

    ; 'return', but this should be to the instruction we replaced with BREAK.
    ret

_debug_args:
_debug_args_address:
    defs    2 ; Address
_debug_args_af:
    defs    2 ; AF
_debug_args_bc:
    defs    2 ; BC
_debug_args_de:
    defs    2 ; DE
_debug_args_hl:
    defs    2 ; HL
_debug_args_ix:
    defs    2 ; IX
_debug_args_iy:
    defs    2 ; IY
