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

    ; Get the pointer for the currently-running task.
    ld      HL, (os_running_task)

    ; Move the stack pointer into BC. This is the only way.
    ld      (os_store_sp), SP
    ld      BC, (os_store_sp)

    ; Store the stack pointer for the current task and move
    ; HL onto the stack pointer for the next task.
    ld      (HL), C
    inc     HL
    ld      (HL), B
    inc     HL

    ; Loop back to the beginning if we've hit the end.
    ld      A, H
    cp      (os_tasks_end & $ff00) >> 8
    jp      nz, handler_nmi_skip
    ld      A, L
    cp      os_tasks_end & $00ff
    jp      nz, handler_nmi_skip

    ld      HL, os_tasks

handler_nmi_skip:

    ; Store a pointer for this task.
    ld      (os_running_task), HL

    ; Now load the SP for this task into BC.
    ld      C, (HL)
    inc     HL
    ld      B, (HL)

    ; Move into HL, then into SP.
    ld      L, C
    ld      H, B
    ld      SP, HL

    pop     DE
    pop     HL
    pop     BC
    pop     AF
    
    retn

    org $0200
main:
    ; OS stack initialization.
    ld      SP, $8fff

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

    ; Initialize the OS variables that need initializing.
    ld      A, 0
    ld      B, 16
    ld      HL, os_tasks

    call    memset

    ; Load the tasks into RAM.
    ld      BC, task_A_end - task_A
    ld      HL, task_A
    call    os_spawn

    ld      BC, task_B_end - task_B
    ld      HL, task_B
    call    os_spawn

    ; Start the timer.
    ld      A, $01
    out     (14), A

    ; Start executing task A.
    ld      HL, os_tasks
    call    os_exec

; C stdlib-style helper functions, e.g. memset, strcmp, etc.

; Set a region of memory to a given value.
;
; Parameters:
;   A - value to set
;   B - byte count
;   HL - pointer to memory region
memset:
    ld      (HL), A
    inc     HL
    djnz    memset

    ret

; OS-specific functions.

; Spawn a process using the given image.
;
; Parameters:
;   HL - pointer to the image.
;   BC - size of the image.
os_spawn:
    ; Get a pointer to the next available memory region.
    call    os_next_task_memory

    ; Mark this region as used.
    ld      A, $ff
    ld      (DE), A
    inc     DE

    ; Save DE - we'll need it again later.
    push    DE

    ; DE now holds the memory region.
    ; Load the process into RAM.
    ldir

    ; Now set up the stack.

    ; Restore the pointer to the entry point into HL,
    ; then save it again.
    pop     HL
    push    HL

    ; Get the initialization value of SP.
    ld      DE, $0fff
    add     HL, DE

    ; Restore entry point.
    pop     DE

    ; Save OS stack pointer into BC.
    ld      (os_store_sp), SP
    ld      BC, (os_store_sp)

    ; Initialize the process stack:
    ; ENTRY
    ; AF
    ; BC
    ; HL
    ; DE    <- SP

    ; Save onto stack.
    ld      SP, HL
    push    DE

    ; Set up registers.
    ld      HL, 0
    push    HL
    push    HL
    push    HL
    push    HL

    ; Store the stack pointer in the relevant task entry.
    ld      (os_store_sp), SP
    ld      DE, (os_store_sp)

    ld      (os_store_sp), BC
    ld      SP, (os_store_sp)

    ; Get a pointer to the next task entry.
    call    os_next_task_entry

    ; Next entry is now in HL.
    ld      (HL), E
    inc     HL
    ld      (HL), D

    ; We've initialized the process, now return.
    ret

; Begin executing the given process.
; The process must not already be running.
;
; Parameters:
;   HL - pointer to the process to be executed.
os_exec:
    inc     SP
    inc     SP

    ld      (os_running_task), HL

    ld      E, (HL)
    inc     HL
    ld      D, (HL)

    ld      L, E
    ld      H, D
    ld      SP, HL

    pop     DE
    pop     HL
    pop     BC
    pop     AF

    ret

; Get the next free memory section that can hold a process image.
;
; Parameters: None
;
; Returns: A pointer to the region, in DE.
os_next_task_memory:
    push    HL
    push    BC

    ld      HL, $9000
    ld      BC, $1000

    ld      A, (HL)
    cp      $00

    jp      z, os_next_task_memory_done

    add     HL, BC

os_next_task_memory_done:
    ; Move the pointer into DE.
    push    HL
    pop     DE

    pop     BC
    pop     HL

    ret

; Get the next process entry available.
;
; Parameters: None
;
; Returns: A pointer to the entry, in HL.
os_next_task_entry:
    ld      HL, os_tasks

os_next_task_entry_loop:
    ld      A, (HL)
    cp      0
    jp      nz, os_next_task_entry_next_1

    inc     HL

    ld      A, (HL)
    cp      0
    jp      nz, os_next_task_entry_next_2

    ; At this point we've found a null pointer.
    ; Decrement HL and return.
    dec     HL
    ret

os_next_task_entry_next_1:
    inc     HL
os_next_task_entry_next_2:
    inc     HL

    jp      os_next_task_entry_loop

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

; Task images.

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
    ld      HL, $01f4
    call    sleep
    
    jp      task_B

task_B_string:
    text    "xyz"
task_B_end:

; OS memory.
    org     $8000
os_store_sp:
    blk     2
os_running_task:
    blk     2
os_tasks:
    blk     4
os_tasks_end:

    org     $9000
task_A_run:
TASK_A_RUN set task_A_run

    org     $a000
task_B_run:
TASK_B_RUN set task_B_run
