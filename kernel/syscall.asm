    ; Definitions
    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000
    
    ; Syscall table.
_syscall_table:
    defw    _do_swrite
    defw    _do_sread

    ; DREAD, DWRITE no longer supported.
    defw    __invalid_syscall
    defw    __invalid_syscall

    defw    _do_fopen
    defw    _do_fread
    defw    _do_fwrite
    defw    _do_fclose

    defw    _do_dinfo
    defw    _do_finfo

    defw    _do_fentries
    defw    _do_fentry

    defw    _do_pexec

    defw    _do_sighandle

    defw    _do_fdelete
    
    defw    _do_pload

    defw    _do_smode

    defw    _do_version

    PUBLIC  _syscall_handler

    EXTERN  _status_set_syscall
    EXTERN  _status_clr_syscall

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

    ; Check:
    ; * That value in A is even.
    bit     0, A
    jp      nz, __invalid_syscall

    push    AF
    call    _status_set_syscall
    pop     AF

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

    EXTERN  _startup_flags

    ; Executed when we see an invalid syscall.
    ; Performs a warm restart of the kernel.
__invalid_syscall:
    ld      A, $01
    ld      (_startup_flags), A
    jp      $0000

_syscall_common_ret:
    push    AF
    push    HL
    push    BC
    push    DE

    call    _status_clr_syscall

    pop     DE
    pop     BC
    pop     HL
    pop     AF

    ret

    ; *****************************
    ; SYSCALL DEFINITIONS
    ; *****************************

    EXTERN   _tx_buf

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

    ; Preserve character to send because we're going to need
    ; L.
    ld      B, L

__swrite_wait_available_loop:
    ; This needs to be an atomic operation; Disable interrupts.
    di

    ld      HL, (_tx_buf_tail)
    ex      DE, HL
    ld      HL, (_tx_buf_head)

    ; Tail in DE, head in HL.

    ; If head is one less than tail,
    ; we need to wait until that isn't the case.
__swrite_is_available:
    inc     L

    ; Only need to compare lower half.
    ; Buffer is 256-byte aligned.
    ld      A, E
    cp      L
    jp      nz, __swrite_continue
    
    ei
    nop
    jp      __swrite_wait_available_loop

__swrite_continue:
    ; Otherwise decrement L and continue.
    dec     L

    ; If head and tail are equal, we need to enable
    ; interrupts before appending to the buffer.
    ld      A, E
    cp      L
    jp      nz, __swrite_append

    ; Enable TX interrupts
    ld      A, 0b10110110
    out     (UART_PORT_CONTROL), A

__swrite_append:
    ; Write into buffer. Increment lower half of head.
    ; This causes wraparound automatically.
    ld      (HL), B
    inc     L
    ld      (_tx_buf_head), HL

__swrite_done:
    ei

    pop     BC
    pop     DE
    jp      _syscall_common_ret

    PUBLIC  _tx_buf_head
    PUBLIC  _tx_buf_tail

_tx_buf_head:
    defw    _tx_buf
_tx_buf_tail:
    defw    _tx_buf

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
    jp      _syscall_common_ret

    PUBLIC  _rx_buf_offs_head
    PUBLIC  _rx_buf_offs_tail
    PUBLIC  _rx_buf

_rx_buf_offs_head:
    defb    0
_rx_buf_offs_tail:
    defb    0

_rx_buf:
    defs    256

    EXTERN  _disk_read
    EXTERN  _disk_write

    EXTERN  _file_open
    EXTERN  _file_read
    EXTERN  _file_write
    EXTERN  _file_close
    EXTERN  _file_delete

    ; 4: fopen: Open a file for writing.
    ;
    ; Parameters:
    ; DE - pointer to filename string
    ; C - mode for opening
    ;
    ; Returns:
    ; File descriptor/error code in HL.
    ;
    ; Error codes are <0, valid file descriptors are >=0.
    ;
    ; Possible error codes are:
    ; E_FILENOTFOUND = -1
    ; E_FILELIMIT = -2
_do_fopen:
    ; BC, DE is already on stack (in that order).
    ; We should be able to just call the function.
    call    _file_open

    ; Restore BC, DE
    pop     BC
    pop     DE
    
    ; Increment SP. We want to skip past the saved HL on the stack,
    ; rather than overwriting the return value of file_fopen.
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    ; 5: fread: Read an opened file.
    ;
    ; Parameters:
    ; HL - pointer to location to write to
    ; DE - number of bytes to read
    ; BC - file descriptor
    ;
    ; Returns:
    ; Number of bytes written in HL.
_do_fread:
    ; BC, DE, HL are already on stack.
    call    _file_read

    ; Restore BC, DE
    pop     BC
    pop     DE

    ; Increment SP, rather than restoring HL and trashing the return value.
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    ; 6: fwrite: Write an opened file.
    ;
    ; Parameters:
    ; HL - pointer to location to read to
    ; DE - number of bytes to write
    ; BC - file descriptor
    ;
    ; Returns:
    ; Number of bytes written in HL.
_do_fwrite:
    ; BC, DE, HL are already on stack.
    call    _file_write

    ; Restore BC, DE
    pop     BC
    pop     DE

    ; Increment SP, rather than restoring HL and trashing the return value.
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    ; 7: fclose: Close an opened file.
    ;
    ; Parameters:
    ; BC - file descriptor
_do_fclose:
    ; BC is already top of stack.
    call    _file_close

    ; Restore BC, DE, HL
    pop     BC
    pop     DE
    pop     HL

    jp      _syscall_common_ret

    EXTERN  _disk_info

    ; 8: dinfo: Get information about disk.
    ;
    ; Parameters: None
    ;
    ; Returns:
    ; Pointer to disk info struct, in HL.
_do_dinfo:
    pop     BC
    pop     DE
    inc     SP
    inc     SP

    ld      HL, _disk_info

    jp      _syscall_common_ret

    EXTERN  _file_info

    ; 9: finfo: Get information about a file.
    ;
    ; Parameters:
    ; BC - Pointer to struct in which to store the file details
    ; DE - Pointer to filename string
    ;
    ; Returns:
    ; 0 on success, error code on failure.
_do_finfo:
    ; BC and DE are already on top of stack.
    call    _file_info

    pop     BC
    pop     DE
    
    ; Don't want to trash return value in HL.
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    EXTERN  _file_entries

    ; 10: fentries: Return number of file entries in the root directory.
    ;
    ; Parameters: None
    ;
    ; Returns:
    ; The number of file entries.
_do_fentries:
    call    _file_entries

    pop     BC
    pop     DE
    inc     SP
    inc     SP
    
    jp      _syscall_common_ret

    EXTERN  _file_entry

    ; 11: fentry: Get the name of the nth entry in the root directory.
    ;
    ; Parameters:
    ; BC - n (entry index)
    ; DE - Pointer to string to populate
    ;
    ; Returns:
    ; (int) 0 on success, error code on failure
_do_fentry:
    ; BC and DE are already on top of stack.
    call    _file_entry

    pop     BC
    pop     DE
    inc     SP
    inc     SP
    
    jp      _syscall_common_ret

    EXTERN  _process_exec
    EXTERN  _signal_sethandler

    ; 12: pexec: Execute loaded executable image.
    ;
    ; Parameters:
    ; BC - argc
    ; DE - argv
    ; HL - address
    ;
    ; Returns:
    ; (int) exit code of executable.
_do_pexec:
    ; HL, BC and DE are on top of stack.
    call    _process_exec

    ; Restore BC, DE, but not HL (return value).
    pop     BC
    pop     DE
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    ; 13: sighandle: Set handler function for signal.
    ;
    ; Parameters:
    ; BC - signal number
    ; DE - function pointer for handler
_do_sighandle:
    ; BC, DE already on TOS.
    call    _signal_sethandler

    pop     BC
    pop     DE
    pop     HL

    jp      _syscall_common_ret

    ; 14: fdelete: Delete a file.
    ;
    ; Parameters:
    ; BC - pointer to filename
    ;
    ; Returns:
    ; Error code for delete operation, or 0 if successful.
_do_fdelete:
    ; BC already on TOS.
    call    _file_delete

    pop     BC
    pop     DE
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    EXTERN  _process_load

    ; 15: pload: Load an executable
    ;
    ; Parameters:
    ; DE - pointer to address
    ; BC - pointer to filename
    ;
    ; Returns:
    ; Error code for load operation, or 0 if successful.
_do_pload:
    ; BC, DE already on TOS.
    call    _process_load

    pop     BC
    pop     DE
    inc     SP
    inc     SP

    jp      _syscall_common_ret

    EXTERN  _serial_mode

    ; 16: smode: Set serial mode
    ;
    ; Parameters:
    ; C - mode
    ;
    ; Returns:
    ; Nothing.
_do_smode:
    ; Needs to be an atomic operation.
    di

    ; BC already on TOS.
    call    _serial_mode

    pop     BC
    pop     DE
    pop     HL

    ei

    jp      _syscall_common_ret

    ; 17: version: Get version string of kernel.
    ;
    ; Parameters:
    ; None.
    ;
    ; Returns:
    ; Pointer to version in HL.
_do_version:
    pop     BC
    pop     DE
    pop     HL

    ld      HL, __kernel_version
    jp      _syscall_common_ret

__kernel_version:
    defm    "0.2.2", 0

__test:
    defm    "CANCEL handler: %04x\n\r", 0
