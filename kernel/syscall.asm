    ; Definitions
    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000

    EXTERN  _driver_6850_tx
    EXTERN  _serial_mode

    EXTERN  _file_open
    EXTERN  _file_read
    EXTERN  _file_write
    EXTERN  _file_close
    EXTERN  _file_delete
    EXTERN  _file_info
    EXTERN  _file_entries
    EXTERN  _file_entry

    EXTERN  _process_spawn
    EXTERN  _process_load
    EXTERN  _scheduler_state
    EXTERN  _process_exit
    EXTERN  _scheduler_exitcode

    EXTERN  _signal_sethandler

    ; Syscall mappings.
    defc    _do_swrite = _driver_6850_tx
    ; sread
    defc    _do_smode = _serial_mode

    ; dinfo

    defc    _do_fopen = _file_open
    defc    _do_fread = _file_read
    defc    _do_fwrite = _file_write
    defc    _do_fclose = _file_close
    defc    _do_fdelete = _file_delete
    defc    _do_finfo = _file_info
    defc    _do_fentries = _file_entries
    defc    _do_fentry = _file_entry

    defc    _do_pspawn = _process_spawn
    defc    _do_pload = _process_load
    defc    _do_pstate = _scheduler_state
    ; pexit
    defc    _do_pexitcode = _scheduler_exitcode

    defc    _do_sighandle = _signal_sethandler

    ; sysinfo

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

    defw    _do_pspawn

    defw    _do_sighandle

    defw    _do_fdelete
    
    defw    _do_pload

    defw    _do_smode

    defw    _do_sysinfo

    defw    _do_pstate
    defw    _do_pexit
    defw    _do_pexitcode

    PUBLIC  _syscall_handler

    EXTERN  _status_set_syscall
    EXTERN  _status_clr_syscall

    ; Syscall handler.
    ;
    ; Calculates absolute position of syscall function address
    ; using offset provided in A, then executes the function at
    ; that address.
_syscall_handler:
    di

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
    call    __execute_syscall

    ; Restore BC, DE, but preserve
    ; return value in HL.
    pop     BC
    pop     DE
    inc     SP
    inc     SP

__syscall_ret:
    push    AF
    push    HL
    push    BC
    push    DE

    call    _status_clr_syscall

    pop     DE
    pop     BC
    pop     HL
    pop     AF

    ei

    ret

__execute_syscall:
    jp      (HL)

    EXTERN  _startup_flags

    ; Executed when we see an invalid syscall.
    ; Performs a warm restart of the kernel.
__invalid_syscall:
    ld      A, $01
    ld      (_startup_flags), A
    jp      $0000

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
    push    DE
    push    HL

    ld      A, (_rx_buf_offs_head)
    ld      E, A

    ; Wait for char in buffer.
    ei
__sread_wait:
    ld      A, (_rx_buf_offs_tail)
    cp      E

    ; If head and tail are equal, there's no data in buffer.
    jp      z, __sread_wait
    di

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

    EXTERN  _disk_info

    ; 8: dinfo: Get information about disk.
    ;
    ; Parameters: None
    ;
    ; Returns:
    ; Pointer to disk info struct, in HL.
_do_dinfo:
    ld      HL, _disk_info
    ret

    ; 17: sysinfo: Get info about kernel.
    ;
    ; Parameters:
    ; None.
    ;
    ; Returns:
    ; Pointer to sysinfo struct in HL.
_do_sysinfo:
    ld      HL, _sysinfo
    ret

    PUBLIC  _sysinfo

    ; 19: pexit: Exit current process.
    ;
    ; Parameters:
    ; BC: exit code.
    ;
    ; Returns:
    ; Nothing.
_do_pexit:
    ld      HL, 2
    add     HL, SP
    ld      C, (HL)
    inc     HL
    ld      B, (HL)

    push    BC
    
    call    _process_exit

    ; This syscall doesn't return.
    ; Set return address to the loop function
    ; and return.
    ld      HL, __pexit_loop
    push    HL
    ld      HL, __syscall_ret
    push    HL

    ret

__pexit_loop:
    jp      __pexit_loop

_sysinfo:
    defw    __kernel_version
__sysinfo_numbanks:
    defw    0

__kernel_version:
    defm    "0.4.0", 0

__test:
    defm    "CANCEL handler: %04x\n\r", 0
