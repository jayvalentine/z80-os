; Z80-OS
;
; Copyright (c) 2020 Jay Valentine
;
; Z80-OS is a multitasking operating system for the Z80 CPU.

    org $0000
start:
    jp      main

    org $0100
main:
    ; Register initialization
    ld      SP, $ffff

    ld      HL, prompt
    ld      B, 6

    call    print

    halt

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
