    ; Definitions
    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000
    
    ; Syscall table.
_syscall_table:
    defw    _do_swrite
    defw    _do_sread
    defw    _do_dwrite
    defw    _do_dread
    defw    _do_fopen

    PUBLIC  _syscall_handler

    ; Syscall handler.
    ;
    ; Calculates absolute position of syscall function address
    ; using offset provided in A, then executes the function at
    ; that address.
_syscall_handler:
    ; Save HL, DE, BC
    push    HL
    push    DE
    push    BC

    ; Calculate position of word in syscall table.
    ld      HL, _syscall_table
    ld      D, 0
    ld      E, A
    add     HL, DE
    
    ; Load word into HL.
    ld      E, (HL)
    inc     HL
    ld      D, (HL)
    ex      DE, HL
    
    ; Execute syscall.
    ; Syscall function will need to pop DE, HL off the stack.
    jp      (HL)

    ; *****************************
    ; SYSCALL DEFINITIONS
    ; *****************************

    ; 0: swrite: Write character to serial port.
    ;
    ; Parameters:
    ; L     - Byte character to send.
    ;
    ; Returns:
    ; Nothing.
    ;
    ; Description:
    ; Busy-waits until serial port is ready to transmit, then
    ; writes the given character to the serial port.
_do_swrite:
    pop     BC
    pop     DE
    pop     HL

    push    DE
    push    BC

    ; This needs to be an atomic operation; Disable interrupts.
    di

    ; Preserve character to send because we're going to need
    ; L.
    ld      B, L

    ld      A, (_tx_buf_offs_head)
    ld      E, A
    ld      A, (_tx_buf_offs_tail)

    ; If head and tail are equal, we need to enable interrupts before appending to the buffer.
    cp      E
    jp      nz, __swrite_append

    ; Enable TX interrupts
    ld      A, 0b10110110
    out     (UART_PORT_CONTROL), A

__swrite_append:
    ; Calculate offset into buffer.
    ld      HL, _tx_buf
    ld      D, 0
    add     HL, DE

    ; Write into buffer.
    ld      (HL), B

__swrite_increment:
    ; Increment head. Because the offset is a single byte, wraparound happens automatically.
    ld      HL, _tx_buf_offs_head
    inc     (HL)

__swrite_done:
    ei

    pop     BC
    pop     DE
    ret

    PUBLIC  _tx_buf_offs_head
    PUBLIC  _tx_buf_offs_tail
    PUBLIC  _tx_buf

_tx_buf_offs_head:
    defb    0
_tx_buf_offs_tail:
    defb    0

_tx_buf:
    defs    256

    ; 1: sread: Read character from serial port.
    ;
    ; Parameters:
    ; None.
    ;
    ; Returns:
    ; Character received from serial port, in A.
    ;
    ; Description:
    ; Busy-waits until serial port receives data,
    ; then returns a single received character.
_do_sread:
    pop     BC
    pop     DE
    pop     HL

    push    DE
    push    HL

    ld      A, (_rx_buf_offs_head)
    ld      E, A

    ; Wait for char in buffer.
__sread_wait:
    ld      A, (_rx_buf_offs_tail)
    cp      E

    ; If head and tail are equal, there's no data in buffer.
    jp      z, __sread_wait

__sread_available:
    ; Calculate offset into buffer.
    ld      HL, _rx_buf
    ld      D, 0
    add     HL, DE

    ; Load character into A.
    ld      A, (HL)

    ; Increment head.
    ld      HL, _rx_buf_offs_head
    inc     (HL)

    pop     HL
    pop     DE
    ret

    PUBLIC  _rx_buf_offs_head
    PUBLIC  _rx_buf_offs_tail
    PUBLIC  _rx_buf

_rx_buf_offs_head:
    defb    0
_rx_buf_offs_tail:
    defb    0

_rx_buf:
    defs    256

    EXTERN  _disk_wait_cmd
    EXTERN  _disk_set_lba
    EXTERN  _disk_write_data
    EXTERN  _disk_read_data

    defc    DISKPORT = $18

    ; 2: dwrite: Write 512 bytes to disk sector.
    ;
    ; Parameters:
    ; Sector number in DEBC
    ; Location of buffer to write from in HL.
    ;
    ; Returns:
    ; Nothing;
    ;
    ; Description:
    ; Writes a sector to the CF-card disk,
    ; from the buffer pointed to by HL.
_do_dwrite:
    pop     BC
    pop     DE

    ; Sector number now in DEBC, so we just need
    ; to call the set_lba subroutine.
    call    _disk_wait_cmd
    call    _disk_set_lba

    ; Transfer one sector
    ld      A, $01
    out     (DISKPORT+2), A

    ; Write sector command.
    ld      A, $30
    out     (DISKPORT+7), A

    pop     HL

    ; Write 512 bytes to CF-card.
    call    _disk_write_data

    ret

    ; 3: dread: Read 512 bytes from disk sector.
    ;
    ; Parameters:
    ; Sector number in DEBC
    ; Location of buffer to read to in HL.
    ;
    ; Returns:
    ; Nothing;
    ;
    ; Description:
    ; Reads a sector from the CF-card disk,
    ; into the buffer pointed to by HL.
_do_dread:
    pop     BC
    pop     DE

    ; Sector number in DEBC, so we just need
    ; to call the set_lba subroutine.
    call    _disk_wait_cmd
    call    _disk_set_lba

    ; Transfer one sector
    ld      A, $01
    out     (DISKPORT+2), A

    ; Read sector command.
    ld      A, $20
    out     (DISKPORT+7), A
    
    pop     HL

    ; Read 512 bytes from CF-card.
    call    _disk_read_data

    ret

    EXTERN  _file_fopen

    ; 4: fopen: Open a file for writing.
    ;
    ; Parameters:
    ; DE - pointer to filename string
    ; C - mode for opening
_do_fopen:
    ; BC, DE is already on stack (in that order).
    ; We should be able to just call the function.
    call    _file_fopen

    ; Restore BC, DE, HL
    pop     BC
    pop     DE
    pop     HL

    ret
