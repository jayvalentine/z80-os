    ; Interface to kernel.
    PUBLIC  _kernel
    PUBLIC  _set_reg
    PUBLIC  _ram_test

    PUBLIC  _reg_state

    ; void kernel(void)
_kernel:
    jp      $0000

    ; void set_reg(ubyte val) __z88dk_fastcall
_set_reg:
    ld      A, L
    ld      (_reg_state), A
    out     ($80), A
    ret

_ram_test:
    ; Write a known pattern into RAM.
    ld      HL, $0000
    ld      DE, 256

__write_loop_outer:
    ; Toggle LED.
    ld      A, (_reg_state)
    xor     %00000010
    push    HL
    ld      L, A
    call    _set_reg
    pop     HL

    ; Write test pattern to this block.
    ld      B, 128
__write_loop_inner:
    ld      (HL), E
    inc     HL
    djnz    __write_loop_inner

    dec     DE
    ld      A, D
    or      E
    jp      nz, __write_loop_outer

    ; Switch to a different LED to indicate read.
    ld      L, %11111011
    call    _set_reg

    ; Read from RAM and check that we see the pattern.
    ld      HL, $0000
    ld      DE, 256

__read_loop_outer:
    ; Toggle LED.
    ld      A, (_reg_state)
    xor     %00000100
    push    HL
    ld      L, A
    call    _set_reg
    pop     HL

    ; Read this block.
    ld      B, 128
__read_loop_inner:
    ld      A, (HL)
    cp      E
    jp      nz, __done

    inc     HL

    djnz    __read_loop_inner

    dec     DE
    ld      A, D
    or      E
    jp      nz, __read_loop_outer

__done:
    ret

_reg_state:
    defs    1
