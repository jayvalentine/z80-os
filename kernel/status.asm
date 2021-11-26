    ; Note: Values are inverted. A 1 in a position of
    ; the status register means the corresponding LED
    ; is OFF, and a 0 means it is ON.
    ;
    ; Therefore when we "set" a status LED, we *clear*
    ; the corresponding bit, which is a bit counter-intuitive.
    
    ; Initial value of status flags.
    DEFC    STATUS_INIT = %11111111
    
    ; INT status is bit 1 of the status register.
    DEFC    STATUS_SETMASK_INT = %00000010
    DEFC    STATUS_CLRMASK_INT = %11111101

    ; SYSCALL status is bit 2 of the status register.
    DEFC    STATUS_SETMASK_SYSCALL = %00000100
    DEFC    STATUS_CLRMASK_SYSCALL = %11111011

    ; DISK status is bit 3 of the status register.
    DEFC    STATUS_SETMASK_DISK = %00001000
    DEFC    STATUS_CLRMASK_DISK = %11110111

    PUBLIC  _status_init
    PUBLIC  _status_set_int
    PUBLIC  _status_clr_int
    PUBLIC  _status_set_syscall
    PUBLIC  _status_clr_syscall
    PUBLIC  _status_set_disk
    PUBLIC  _status_clr_disk

    ; Current status of the LEDs.
_current_status:
    defb    0

    ; void status_init(void)
_status_init:
    push    AF
    ld      A, STATUS_INIT
    ld      (_current_status), A
    out     ($80), A
    pop     AF
    ret

    ; void status_clr_int(void)
_status_clr_int:
    push    AF
    ld      A, (_current_status)        ; Get current status
    or      A, STATUS_SETMASK_INT       ; OR mask to set INT bit (LED off)
    ld      (_current_status), A        ; Write back to keep track

    out     ($80), A ; Output to port.

    pop     AF
    ret

    ; void status_clr_syscall(void)
_status_clr_syscall:
    push    AF
    ld      A, (_current_status)        ; Get current status
    or      A, STATUS_SETMASK_SYSCALL   ; OR mask to set SYSCALL bit (LED off)
    ld      (_current_status), A        ; Write back to keep track

    out     ($80), A ; Output to port.
    
    pop     AF
    ret

    ; void status_clr_disk(void)
_status_clr_disk:
    push    AF
    ld      A, (_current_status)        ; Get current status
    or      A, STATUS_SETMASK_DISK      ; OR mask to set DISK bit (LED off)
    ld      (_current_status), A        ; Write back to keep track

    out     ($80), A ; Output to port.
    
    pop     AF
    ret

    ; void status_set_int(void)
_status_set_int:
    push    AF
    ld      A, (_current_status)        ; Get current status
    and     A, STATUS_CLRMASK_INT       ; AND mask to clear INT bit (LED on)
    ld      (_current_status), A        ; Write back to keep track

    out     ($80), A ; Output to port.

    pop     AF
    ret

    ; void status_set_syscall(void)
_status_set_syscall:
    push    AF
    ld      A, (_current_status)        ; Get current status
    and     A, STATUS_CLRMASK_SYSCALL   ; AND mask to clear SYSCALL bit (LED on)
    ld      (_current_status), A        ; Write back to keep track

    out     ($80), A ; Output to port.

    pop     AF
    ret

    ; void status_set_disk(void)
_status_set_disk:
    push    AF
    ld      A, (_current_status)        ; Get current status
    and     A, STATUS_CLRMASK_DISK      ; AND mask to clear DISK bit (LED on)
    ld      (_current_status), A        ; Write back to keep track

    out     ($80), A ; Output to port.

    pop     AF
    ret
