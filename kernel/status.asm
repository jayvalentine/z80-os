    ; Note: Values are inverted. A #1 in a position of
    ; the status register means the corresponding LED
    ; is OFF, and a #0 means it is ON.
    ;
    ; Therefore when we "set" a status LED, we *clear*
    ; the corresponding bit, which is a bit counter-intuitive.
    
    ; Initial value of status flags.
    .equ    STATUS_INIT, 0b11111111
    
    ; INT status is bit #1 of the status register.
    .equ    STATUS_SETMASK_INT, 0b00000010
    .equ    STATUS_CLRMASK_INT, 0b11111101

    ; KERNEL status is bit #2 of the status register.
    .equ    STATUS_SETMASK_KERNEL, 0b00000100
    .equ    STATUS_CLRMASK_KERNEL, 0b11111011

    ; DISK status is bit #3 of the status register.
    .equ    STATUS_SETMASK_DISK, 0b00001000
    .equ    STATUS_CLRMASK_DISK, 0b11110111

    .globl  _status_init
    .globl  _status_set_int
    .globl  _status_clr_int
    .globl  _status_set_kernel
    .globl  _status_clr_kernel
    .globl  _status_is_set_kernel
    .globl  _status_set_disk
    .globl  _status_clr_disk

    ; Current status of the LEDs.
_current_status:
    .byte   #0

    ; void status_init(void)
_status_init:
    push    AF
    ld      A, #STATUS_INIT
    ld      (_current_status), A
    out     (0x80), A
    pop     AF
    ret

    ; void status_clr_int(void)
_status_clr_int:
    push    AF
    ld      A, (_current_status)        ; Get current status
    or      A, #STATUS_SETMASK_INT       ; OR mask to set INT bit (LED off)
    ld      (_current_status), A        ; Write back to keep track

    out     (0x80), A ; Output to port.

    pop     AF
    ret

    ; void status_clr_kernel(void)
_status_clr_kernel:
    push    AF
    ld      A, (_current_status)        ; Get current status
    or      A, #STATUS_SETMASK_KERNEL   ; OR mask to set KERNEL bit (LED off)
    ld      (_current_status), A        ; Write back to keep track

    out     (0x80), A ; Output to port.
    
    pop     AF
    ret

    ; void status_clr_disk(void)
_status_clr_disk:
    push    AF
    ld      A, (_current_status)        ; Get current status
    or      A, #STATUS_SETMASK_DISK      ; OR mask to set DISK bit (LED off)
    ld      (_current_status), A        ; Write back to keep track

    out     (0x80), A ; Output to port.
    
    pop     AF
    ret

    ; void status_set_int(void)
_status_set_int:
    push    AF
    ld      A, (_current_status)        ; Get current status
    and     A, #STATUS_CLRMASK_INT       ; AND mask to clear INT bit (LED on)
    ld      (_current_status), A        ; Write back to keep track

    out     (0x80), A ; Output to port.

    pop     AF
    ret

    ; void status_set_kernel(void)
_status_set_kernel:
    push    AF
    ld      A, (_current_status)        ; Get current status
    and     A, #STATUS_CLRMASK_KERNEL   ; AND mask to clear KERNEL bit (LED on)
    ld      (_current_status), A        ; Write back to keep track

    out     (0x80), A ; Output to port.

    pop     AF
    ret

    ; void status_set_disk(void)
_status_set_disk:
    push    AF
    ld      A, (_current_status)        ; Get current status
    and     A, #STATUS_CLRMASK_DISK      ; AND mask to clear DISK bit (LED on)
    ld      (_current_status), A        ; Write back to keep track

    out     (0x80), A ; Output to port.

    pop     AF
    ret

    ; bool status_is_set_kernel(void)
_status_is_set_kernel:
    ; Check if KERNEL flag in current status is set.
    ;
    ; Remember that the current status flags are inverted
    ; (i.e. active LOW).
    ld      A, (_current_status)
    bit     2, A
    jp      z, __status_is_set_kernel_true

    ld      A, #0
    ret

__status_is_set_kernel_true:
    ld      A, #1
    ret
