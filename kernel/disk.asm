    ; *****************************
    ; DISK DRIVERS
    ;
    ; Functions for accessing sectors on the CF-card.
    ; *****************************

    EXTERN  _puts

    defc    DISKPORT = $18

    ; **************************
    ; HIGH-LEVEL ROUTINES
    ;
    ; These subroutines are the high-level
    ; driver routines that a program
    ; uses to read/write the CF-card.
    ; **************************

    PUBLIC  _disk_init

    ; void disk_init(void)
    ;
    ; Initialises CF-card.
_disk_init:
    push    AF
    call    _disk_wait
    ld      A, $04
    out     (DISKPORT+7), A

    call    _disk_wait
    ld      A, $01
    out     (DISKPORT+1), A

    call    _disk_wait
    ld      A, $ef
    out     (DISKPORT+7), A

    call    _disk_chkerr
    pop     AF
    ret

    ; **************************
    ; DATA TRANSFER ROUTINES
    ;
    ; These subroutines transfer blocks to or from
    ; the CF-card.
    ; **************************

    PUBLIC  _disk_read_data
    PUBLIC  _disk_write_data

    ; Reads 512 bytes (one sector) from the CF card.
    ; Reads the data into the location pointed to by HL.
    ; Assumes a read command has been previously initiated.
_disk_read_data:
    push    AF
    push    BC
    push    HL

    call    _disk_wait_data
    call    _disk_chkerr

    ld      C, DISKPORT
    ld      B, 0

    ; Load 512 bytes into HL.
    inir
    inir

__read_data_done:
    pop     HL
    pop     BC
    pop     AF
    ret

    ; Writes 512 bytes (one sector) to the CF card.
    ; Writes the data from the location pointed to by HL.
    ; Assumes a write command has been previously initiated.
_disk_write_data:
    push    AF
    push    BC
    push    HL

    call    _disk_wait_data

    ld      C, DISKPORT
    ld      B, 0

    ; Write 512 bytes from HL.
    otir
    otir

    call    _disk_wait_cmd
    call    _disk_chkerr

__write_data_done:
    pop     HL
    pop     BC
    pop     AF
    ret

    ; **************************
    ; UTILITY ROUTINES
    ;
    ; Shorthands for functionality of the CF-card.
    ; **************************

    PUBLIC  _disk_set_lba

    ; Set the LBA for the CF-card, stored as a 28-bit value
    ; in DEBC (the top 4 bits of D are ignored).
_disk_set_lba:
    push    AF

    ; Set the lower 3/4ths of the LBA via registers 3-5.
    ld      A, C
    out     (DISKPORT+3), A
    ld      A, B
    out     (DISKPORT+4), A
    ld      A, E
    out     (DISKPORT+5), A
    
    ; Special handling for register 6, as only the bottom half is used
    ; for LBA.
    ld      A, D

    ; We only care about the bottom half of this top byte.
    and     A, %00001111

    ; Master, LBA mode.
    or      A, %11100000
    out     (DISKPORT+6), A
    
    pop     AF
    ret

    ; **************************
    ; WAIT ROUTINES
    ;
    ; These subroutines implement waits for various
    ; conditions of the CF-card.
    ; **************************

    PUBLIC  _disk_wait
    PUBLIC  _disk_wait_data
    PUBLIC  _disk_wait_cmd

_disk_wait:
    in      A, (DISKPORT+7)
    and     %10000000
    jp      nz, _disk_wait

    ret

_disk_wait_data:
    in      A, (DISKPORT+7)
    and     %10001000
    xor     %00001000
    jp      nz, _disk_wait_data

    ret

_disk_wait_cmd:
    in      A, (DISKPORT+7)
    and     %11000000
    xor     %01000000
    jp      nz, _disk_wait_cmd

    ret

    ; **************************
    ; ERROR CHECKING ROUTINES
    ;
    ; These subroutines check for and report
    ; errors from the CF-card.
    ; **************************
    PUBLIC  _disk_chkerr

_disk_chkerr:
    in      A, (DISKPORT+7)
    bit     0, A
    jp      z, __chkerr_noerr

    ld      HL, _disk_error
    call    _puts

__chkerr_noerr:
    ret

    ; **************************
    ; DATA
    ; **************************
_disk_error:
    defm    "Error in CF-card!\n\r"
    defb    0