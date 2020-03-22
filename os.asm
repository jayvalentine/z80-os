; Z80-OS
;
; Copyright (c) 2020 Jay Valentine
;
; Z80-OS is a multitasking operating system for the Z80 CPU.

    org $0000
start:
    jp      main

    org $0066
handler_nmi:
    ld      HL, prompt
    ld      B, 6

    call    print

    ; Restart the timer.
    ld      A, $01
    out     (14), A
    
    retn

    org $0100
main:
    ; Register initialization
    ld      SP, $ffff

    ; Timer initialization.
    ; 1s interval - 8,000,000 cycles @ 8MHz.
    ; Timer value needs to be 0x7a1200.
    ld      A, $00
    out     (10), A
    ld      A, $12
    out     (11), A
    ld      A, $7a
    out     (12), A
    ld      A, $00
    out     (13), A

    ; Start the timer.
    ld      A, $01
    out     (14), A

    ; Infinite loop.
loop:
    jp      loop

; Prints a text string, followed by a newline.
;
; Parameters:
;   HL - pointer to the string
;   B  - string length
print:
    ; Print a character, increment to the next one.
    ld      A, (HL)
    inc     HL
    out     (0), A

    ; Loop until all characters have been printed.
    djnz    print

    ; Print a newline.
    ld      A, $0a
    out     (0), A
    ld      A, $0d
    out     (0), A

    ret

prompt:
    text    "Z80-OS"
