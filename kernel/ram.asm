    ; Routines for handling banked RAM.

    ; I/O port for bank select register.
    defc    BANK_SELECT = $30
    defc    BANKED_MEM_END = $ffff

_dst_bank:
    defs    1
_byte:
    defs    1

    ; void ram_copy(char * dst, uint8_t bank, char * src, size_t n)
    ;
    ; Copies data from one RAM bank to another.
    ;
    ; NOT REENTRANT.
    PUBLIC  _ram_copy
_ram_copy:
    ld      HL, 2
    add     HL, SP

    ; Get n into BC
    ld      C, (HL)
    inc     HL
    ld      B, (HL)
    inc     HL

    ; Get src into DE
    ld      E, (HL)
    inc     HL
    ld      D, (HL)
    inc     HL

    ; Get bank into A, store for later use.
    ld      A, (HL)
    inc     HL
    inc     HL
    ld      (_dst_bank), A

    ; Get dst into HL.
    ld      A, (HL)
    inc     HL
    ld      H, (HL)
    ld      L, A

    ; NO STACK USAGE BEYOND HERE!

__ram_copy_loop:
    ; Read byte to copy.
    ld      A, (DE)
    ld      (_byte), A

    ; Switch to destination bank.
    ld      A, (_dst_bank)
    out     (BANK_SELECT), A

    ; Write byte to destination (in HL).
    ld      A, (_byte)
    ld      (HL), A

    ; Switch back to original bank.
    ld      A, (_bank_current)
    out     (BANK_SELECT), A

    ; Increment pointers, decrement count.
    inc     HL
    inc     DE
    dec     BC

    ; If count is 0, exit loop.
    ld      A, B
    or      C
    jp      nz, __ram_copy_loop

    ; Done copying, now on original bank.
__ram_copy_done:
    ret

_bank_current:
    defb    0

    ; uint8_t bank_current(void)
    ;
    ; Gets current memory bank.
    PUBLIC  _ram_bank_current
_ram_bank_current:
    ld      A, (_bank_current)
    ld      H, 0
    ld      L, A
    ret

    ; void bank_set(uint8_t bank) __z88dk_fastcall
    ;
    ; Sets the current memory bank.
    PUBLIC  _ram_bank_set
_ram_bank_set:
    ; Bank number in L.
    ld      A, L

    ; Pop return address off stack.
    ; We're going to change the memory bank,
    ; so we want to ensure we can return properly.
    pop     HL

    ld      (_bank_current), A
    out     (BANK_SELECT), A

    jp      (HL)

    ; uint16_t bank_test(void)
    ;
    ; Returns the number of banks.
    PUBLIC  _ram_bank_test
_ram_bank_test:
    ; First set the last byte of each bank to
    ; the expected bank number.
    ld      L, 0

__ram_bank_test_init:
    call    _ram_bank_set
    ld      A, $ff
    ld      (BANKED_MEM_END), A
    inc     L
    jp      nz, __ram_bank_test_init

    ; Now cycle through the banks.
    ; The first time we see a value other
    ; than the one we expect, we've looped back
    ; round, so we know the number of banks.
    ld      L, 0

__ram_bank_test_loop:
    call    _ram_bank_set
    ld      A, (BANKED_MEM_END)
    cp      $ff
    jp      nz, __ram_bank_test_done

    ld      A, L
    ld      (BANKED_MEM_END), A

    inc     L
    jp      nz, __ram_bank_test_loop

    ; L is 0, so we have 256 banks.
    ld      HL, 256
    ret

__ram_bank_test_done:
    ; L holds #banks (less than 256).
    ; Therefore just set H to 0 to form a 16-bit value.
    ld      H, 0
    ret
