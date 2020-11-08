
    PUBLIC  _puts
    PUBLIC  _strcmp
    PUBLIC  _memcpy
    
    ; int puts(const char * str) __z88dk_fastcall
_puts:
    push    HL
    pop     DE

__puts_loop:
    ; src in DE.
    ld      A, (DE)
    inc     DE
    cp      0
    jp      z, __puts_done

    ; Not null, print to serial port.
    ld      L, A
    ld      A, 0
    rst     48

    ; Loop.
    jp      __puts_loop

__puts_done:
    ; Successful, return 0.
    ld      HL, 0
    ret

    ; int strcmp(const char *s1, const char *s2)
_strcmp:
    push    BC
    push    DE
    push    AF

    ; Skip over return address.
    ld      HL, 8
    add     HL, SP

    ; Load s2 into DE (second param).
    ld      E, (HL)
    inc     HL
    ld      D, (HL)
    inc     HL

    ; Load s1 into HL (first param).
    ld      C, (HL)
    inc     HL
    ld      B, (HL)

    ld      H, B
    ld      L, C

__strcmp_compare:
    ld      A, (DE)
    ld      C, (HL)
    cp      C

    ; If non-zero, the strings are different.
    jp      nz, __strcmp_neq

    ; Otherwise, if null they are equal.
    cp      0
    jp      z, __strcmp_eq

    ; Not reached end of string, equal so far.
    ; Increment pointers and loop.
    inc     HL
    inc     DE
    jp      __strcmp_compare

__strcmp_eq:
    ld      HL, 0
    jp      __strcmp_done

__strcmp_neq:
    ld      HL, 1

__strcmp_done:
    pop     AF
    pop     DE
    pop     BC
    ret

    ; void * memcpy(char * dest, const char * src, size_t n)
_memcpy:
    ld      HL, 2
    add     HL, SP

    ; Set IX to start of parameter region.
    push    HL
    pop     IX

    ; n into BC
    ld      C, (IX+0)
    ld      B, (IX+1)

    ; src into HL
    ld      L, (IX+2)
    ld      H, (IX+3)

    ; dest into DE
    ld      E, (IX+4)
    ld      D, (IX+5)

    ; Do the copy.
    ldir

    ; Return value is dest.
    ld      L, (IX+4)
    ld      H, (IX+5)

    ret
