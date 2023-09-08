    ; *****************************
    ; DISK DRIVERS
    ;
    ; Functions for accessing sectors on the CF-card.
    ; *****************************

    .globl  _puts
    .globl  _status_set_disk
    .globl  _status_clr_disk

    .equ    DISKPORT, 0x18

    ; **************************
    ; PUBLIC ROUTINES
    ;
    ; These subroutines are the public
    ; interface to the CF-card.
    ; **************************
    
    .globl  _disk_read
    .globl  _disk_write
    .globl  _disk_init

    ; void disk_init(void)
    ;
    ; Initialises CF-card.
_disk_init:
    push    AF
    call    _disk_wait
    ld      A, #0x04
    out     (DISKPORT+7), A

    call    _disk_wait
    ld      A, #0x01
    out     (DISKPORT+1), A

    call    _disk_wait
    ld      A, #0xef
    out     (DISKPORT+7), A

    call    _disk_chkerr
    pop     AF
    ret

    ; void disk_read(char * buf, uint32_t sector)
    ;
    ; buf    will be in HL
    ; sector will be on stack.
    ;
    ; callee is responsible for cleaning up the stack.
    ;
    ; Reads a sector from CF-card.
_disk_read:
    ; Return address
    pop     IY

    ; Sector
    pop     BC
    pop     DE

    call    _status_set_disk
    
    call    _disk_wait_cmd
    call    _disk_set_lba

    call    _disk_init_read

    ; Read #512 bytes from CF-card.
    call    _disk_read_data

    call    _status_clr_disk

    jp      (IY)



    ; void disk_write(char * buf, uint32_t sector)
    ;
    ; buf    will be in HL
    ; sector will be on stack.
    ;
    ; callee is responsible for cleaning up the stack.
    ;
    ; Writes a sector to CF-card.
_disk_write:
    ; Return address
    pop     IY

    ; Sector
    pop     BC
    pop     DE

    call    _status_set_disk
    
    call    _disk_wait_cmd
    call    _disk_set_lba

    call    _disk_init_write

    ; Write #512 bytes to CF-card.
    call    _disk_write_data

    call    _status_clr_disk

    jp      (IY)

    

    ; **************************
    ; READ/WRITE ROUTINES
    ;
    ; Shorthands for reading/writing sections
    ; from/to CF card.
    ; **************************

    ; Initiates a read from the disk.
_disk_init_read:
    ; Transfer one sector
    ld      A, #0x01
    out     (DISKPORT+2), A

    ; Read sector command.
    ld      A, #0x20
    out     (DISKPORT+7), A

    ret

    ; Initiates a write to the disk.
_disk_init_write:
    ; Transfer one sector
    ld      A, #0x01
    out     (DISKPORT+2), A

    ; Write sector command.
    ld      A, #0x30
    out     (DISKPORT+7), A

    ret

    ; Reads #512 bytes (one sector) from the CF card.
    ; Reads the data into the location pointed to by HL.
    ; Assumes a read command has been previously initiated.
_disk_read_data:
    call    _disk_wait_data
    call    _disk_chkerr

    ld      C, #DISKPORT
    ld      B, #0

    ; Load #512 bytes into HL.
    inir
    inir
    ret

    ; Writes #512 bytes (one sector) to the CF card.
    ; Writes the data from the location pointed to by HL.
    ; Assumes a write command has been previously initiated.
_disk_write_data:
    call    _disk_wait_data

    ld      C, #DISKPORT
    ld      B, #0

    ; Write #512 bytes from HL.
    otir
    otir

    call    _disk_wait_cmd
    call    _disk_chkerr
    
    ret

    ; **************************
    ; UTILITY ROUTINES
    ;
    ; Shorthands for functionality of the CF-card.
    ; **************************

    ; Set the LBA for the CF-card, stored as a #28-bit value
    ; in DEBC (the top #4 bits of D are ignored).
_disk_set_lba:
    push    AF

    ; Set the lower #3/4ths of the LBA via registers #3-5.
    ld      A, C
    out     (DISKPORT+3), A
    ld      A, B
    out     (DISKPORT+4), A
    ld      A, E
    out     (DISKPORT+5), A
    
    ; Special handling for register #6, as only the bottom half is used
    ; for LBA.
    ld      A, D

    ; We only care about the bottom half of this top byte.
    and     A, #0b00001111

    ; Master, #LBA mode.
    or      A, #0b11100000
    out     (DISKPORT+6), A
    
    pop     AF
    ret

    ; **************************
    ; WAIT ROUTINES
    ;
    ; These subroutines implement waits for various
    ; conditions of the CF-card.
    ; **************************

_disk_wait:
    in      A, (DISKPORT+7)
    and     #0b10000000
    jp      nz, #_disk_wait

    ret

_disk_wait_data:
    in      A, (DISKPORT+7)
    and     #0b10001000
    xor     #0b00001000
    jp      nz, #_disk_wait_data

    ret

_disk_wait_cmd:
    in      A, (DISKPORT+7)
    and     #0b11000000
    xor     #0b01000000
    jp      nz, #_disk_wait_cmd

    ret

    ; **************************
    ; ERROR CHECKING ROUTINES
    ;
    ; These subroutines check for and report
    ; errors from the CF-card.
    ; **************************

_disk_chkerr:
    in      A, (DISKPORT+7)
    bit     #0, A
    jp      z, #__chkerr_noerr

    ld      HL, #_disk_error
    call    _puts

__chkerr_noerr:
    ret

    ; **************************
    ; DATA
    ; **************************
_disk_error:
    .asciz  "Error in CF-card!\n\r"
