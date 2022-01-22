    .globl  _break_handler

    .globl  _signal_break

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
    ld      IX, #12
    add     IX, SP
    ld      L, 0(IX)
    ld      H, 1(IX)

    dec     HL ; Decrement return address to get address of rst instruction.
    ld      (_debug_args_address), HL

    ; Save break address and location on stack across function call.
    push    IX
    push    HL

    ; Pass struct to break signal handler.
    ld      HL, #_debug_args
    call    _signal_break

    ; Set return address to address of BREAK instruction.
    pop     HL
    pop     IX
    ld      0(IX), L
    ld      1(IX), H

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
    .ds     #2 ; Address
_debug_args_af:
    .ds     #2 ; AF
_debug_args_bc:
    .ds     #2 ; BC
_debug_args_de:
    .ds     #2 ; DE
_debug_args_hl:
    .ds     #2 ; HL
_debug_args_ix:
    .ds     #2 ; IX
_debug_args_iy:
    .ds     #2 ; IY
