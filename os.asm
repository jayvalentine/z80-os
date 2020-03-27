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
    push    AF
    push    BC
    push    HL
    push    DE

    ; Restart the timer.
    ld      A, $01
    out     (14), A

    pop     DE
    pop     HL
    pop     BC
    pop     AF
    
    retn

    org $0100
main:
    ; Timer initialization.
    ; 1s interval - 400,000 cycles @ 400KHz.
    ; Timer value needs to be 0x00061a80.
    ld      A, $80
    out     (10), A
    ld      A, $1a
    out     (11), A
    ld      A, $06
    out     (12), A
    ld      A, $00
    out     (13), A

    ; Load the tasks into RAM.
    ld      BC, task_A_end - task_A
    ld      HL, task_A
    ld      DE, task_A_run
    ldir

    ld      BC, task_B_end - task_B
    ld      HL, task_B
    ld      DE, task_B_run
    ldir

    ; Initialize SP to point to A's stack.
    ld      SP, $8fff

    ; Start the timer.
    ld      A, $01
    out     (14), A

    ; Start executing task A.
    jp      task_A_run

task_A:
    ld      HL, task_A_string
    ld      B, 3
    call    print

    ; Sleep for 2 seconds.
    ld      HL, $07d0
    call    sleep

    jp      task_A

task_A_string:
    text    "ABC"
task_A_end:

task_B:
    ld      HL, task_B_string
    ld      B, 3
    call    print

    ; Sleep for 2 seconds.
    ld      HL, $07d0
    call    sleep
    
    jp      task_B

task_B_string:
    text    "xyz"
task_B_end:

; Waits for a given amount of time (in milliseconds)
;
; Parameters:
;   HL - the time to wait in milliseconds.
sleep:
    ; Push BC as we're going to trash B.
    push    BC
    ; 1 millisecond is 400 cycles, at 400KHz.

    ; We want to construct an inner loop that runs for 1 millisecond.
    ; Then we just execute that inner loop for as many times as HL.
sleep_outer:
    ; A DJNZ takes ~13 cycles to execute.
    ; So, our inner loop wants to execute roughly 400/13 times.
    ;
    ; Actually, we want to execute one less, to account for the time
    ; the outer jump will take.

    ld      B, 29

sleep_inner:
    djnz    sleep_inner

    ; Decrement HL. Is it 0?
    dec     HL
    ld      A, H
    or      A, L
    jp      nz, sleep_outer

sleep_done:
    ; Pop BC and return.
    pop     BC
    ret

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

    org     $8000
task_A_run:
TASK_A_RUN set task_A_run

    org     $9000
task_B_run:
TASK_B_RUN set task_B_run
