    ; Routines for handling banked RAM.

    ; I/O port for bank select register.
    .equ    BANK_SELECT, 0x30
    .equ    BANKED_MEM_TEST, 0x8000

_dst_bank:
    .ds     #1
_byte:
    .ds     #1

    ; void ram_copy(char * dst, uint8_t bank, char * src, size_t n)
    ;
    ; Copies data from one RAM bank to another.
    ;
    ; NOT REENTRANT.
    ;
    ; TODO: CALLING CONVENTION!!!
    .globl  _ram_copy
_ram_copy:
    push    IX

    ld      HL, #4
    add     HL, SP
    push    HL
    pop     IX

    ; Get n into BC
    ld      C, 5(IX)
    ld      B, 6(IX)

    ; Get src into DE
    ld      E, 3(IX)
    ld      D, 4(IX)

    ; Get bank into A, store for later use.
    ld      A, 2(IX)
    ld      (_dst_bank), A

    ; Get dst into HL.
    ld      L, 0(IX)
    ld      H, 1(IX)

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

    ; If count is #0, exit loop.
    ld      A, B
    or      C
    jp      nz, __ram_copy_loop

    ; Done copying, now on original bank.
__ram_copy_done:
    pop     IX
    ret

_bank_current:
    .byte   0

    ; uint8_t bank_current(void)
    ;
    ; Gets current memory bank.
    .globl  _ram_bank_current
_ram_bank_current:
    ld      A, (_bank_current)
    ret

    ; void bank_set(uint8_t bank)
    ;
    ; Sets the current memory bank.
    .globl  _ram_bank_set
_ram_bank_set:
    ; Pop return address off stack.
    ; We're going to change the memory bank,
    ; so we want to ensure we can return properly.
    pop     HL

    ld      (_bank_current), A
    out     (BANK_SELECT), A

    jp      (HL)

    ; uint8_t bank_test(void)
    ;
    ; Returns the number of banks.
    .globl  _ram_bank_test
_ram_bank_test:
    ; First set the last byte of each bank to
    ; the expected bank number.
    ld      L, #0

__ram_bank_test_init:
    ld      A, L
    out     (BANK_SELECT), A
    ld      A, #0xff
    ld      (BANKED_MEM_TEST), A
    inc     L
    jp      nz, #__ram_bank_test_init

    ; Now cycle through the banks.
    ; The first time we see a value other
    ; than the one we expect, we've looped back
    ; round, so we know the number of banks.
    ld      L, #0

__ram_bank_test_loop:
    ld      A, L
    out     (BANK_SELECT), A
    ld      A, (BANKED_MEM_TEST)
    cp      #0xff
    jp      nz, #__ram_bank_test_done

    ld      A, L
    ld      (BANKED_MEM_TEST), A

    inc     L
    jp      nz, #__ram_bank_test_loop

    ; Return number of banks in A.
    ld      A, L
    ret

__ram_bank_test_done:
    ; Return number of banks in A.
    ld      A, L
    ret
