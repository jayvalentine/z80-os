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

    ; Get return address into HL.
    ld      IX, 12
    add     IX, SP
    ld      L, (IX+0)
    ld      H, (IX+1)

    ; Store location on stack of return address.
    push    IX

    ; Decrement to make HL equal to the address of the BREAK instruction.
    dec     HL
    push    HL

    ; Call break signal handler.
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
