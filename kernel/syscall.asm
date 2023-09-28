    ; Definitions
    .equ    UART_PORT_DATA, #0b00000001
    .equ    UART_PORT_CONTROL, #0b00000000

    .globl  _driver_6850_tx

    .globl  _file_open
    .globl  _file_read
    .globl  _file_write
    .globl  _file_close
    .globl  _file_delete
    .globl  _file_info
    .globl  _file_entries
    .globl  _file_entry

    .globl  _process_spawn
    .globl  _process_load
    .globl  _scheduler_state
    .globl  _process_exit
    .globl  _scheduler_exitcode
    .globl  _process_set_terminal_mode

    .globl  _signal_sethandler

    ; Syscall table.
_syscall_table:
    .word   _driver_6850_tx         ; swrite
    .word   _do_sread               ; sread

    ; DREAD, DWRITE no longer supported.
    .word   __invalid_syscall
    .word   __invalid_syscall

    .word   _file_open               ; fopen
    .word   _file_read               ; fread
    .word   _file_write              ; fwrite
    .word   _file_close              ; fclose

    .word   _do_dinfo                ; dinfo
    .word   _file_info               ; finfo

    .word   _file_entries            ; fentries
    .word   _file_entry              ; fentry

    .word   _process_spawn           ; pspawn

    .word   _signal_sethandler       ; sighandle

    .word   _file_delete             ; fdelete
    
    .word   _process_load            ; pload

    .word   _process_set_terminal_mode ; smode

    .word   _do_sysinfo              ; sysinfo

    .word   _scheduler_state         ; pstate
    .word   _do_pexit                ; pexit
    .word   _scheduler_exitcode      ; pexitcode
    .word   _scheduler_block_current ; pblock

    .globl  _syscall_handler

    .globl  _status_set_syscall
    .globl  _status_clr_syscall

    ; Syscall handler.
    ;
    ; Calculates absolute position of syscall function address
    ; using offset provided in A, then executes the function at
    ; that address.
    ;
    ; TODO: NOT REENTRANT!!!
_syscall_handler:
    di

    ; Save IX as it will be trashed by syscall shim.
    ld      (__syscall_ix), IX

    ; Save HL, DE, BC as they may contain arguments.
    push    HL
    push    DE
    push    BC

    ; Store return address from syscall.
    ; Need to skip the three 16-bit registers on stack.
    ld      HL, #6
    add     HL, SP
    ld      E, (HL)
    inc     HL
    ld      D, (HL)
    ld      (__syscall_ret_address), DE

    ; Set return address to the syscall return handler.
    ld      DE, #__syscall_ret
    ld      HL, #6
    add     HL, SP
    ld      (HL), E
    inc     HL
    ld      (HL), D

    ; Check:
    ; * That value in A is even.
    bit     #0, A
    jp      nz, #__invalid_syscall

    push    AF
    call    _status_set_syscall
    pop     AF

    ; Calculate position of word in syscall table.
    ld      HL, #_syscall_table
    ld      D, #0
    ld      E, A
    add     HL, DE
    
    ; Load word into IX.
    ld      E, (HL)
    inc     HL
    ld      D, (HL)
    push    DE
    pop     IX

    ; Pop registers off stack as they may contain arguments.
    ; Stack needs to be at same position on entry to the syscall function
    ; as on entry to the syscall handler.
    pop     BC
    pop     DE
    pop     HL

    ; Execute syscall.
    ; Return will return to the syscall return handler.
    jp      (IX)

__syscall_ret:
    push    AF
    call    _status_clr_syscall
    pop     AF

    ; Restore IX.
    ld      IX, (__syscall_ix)

    ; Return from syscall.
    ei
    ld      HL, (__syscall_ret_address)
    jp      (HL)

__syscall_ret_address:
    .word   0
__syscall_ix:
    .word   0

    .globl  _startup_flags

    ; Executed when we see an invalid syscall.
    ; Performs a warm restart of the kernel.
__invalid_syscall:
    ld      A, #0x01
    ld      (_startup_flags), A
    rst     8

    .globl  _terminal_available
    .globl  _terminal_get

    ; #1: sread: Read character from serial port.
    ;
    ; Parameters:
    ; None.
    ;
    ; Returns:
    ; Character received from serial port, in DE.
    ;
    ; Description:
    ; Busy-waits until serial port receives data,
    ; then returns a single received character.
_do_sread:
    ; Wait for char in buffer.
    ei
__sread_wait:
    call    _terminal_available

    ; If head and tail are equal, there's no data in buffer.
    jp      z, #__sread_wait
    di

__sread_available:
    ; Get character in A.
    call    _terminal_get

    ; Load character as int into DE (return value).
    ld      E, A
    ld      D, #0
    ret

    .globl  _disk_info

    ; #8: dinfo: Get information about disk.
    ;
    ; Parameters: None
    ;
    ; Returns:
    ; Pointer to disk info struct, in DE.
_do_dinfo:
    ld      DE, #_disk_info
    ret

    ; #17: sysinfo: Get info about kernel.
    ;
    ; Parameters:
    ; None.
    ;
    ; Returns:
    ; Pointer to sysinfo struct in DE.
_do_sysinfo:
    ld      DE, #_sysinfo
    ret

    .globl  _sysinfo

    ; #19: pexit: Exit current process.
    ;
    ; Parameters:
    ; HL: exit code.
    ;
    ; Returns:
    ; Nothing.
_do_pexit:
    call    _process_exit

    ; This syscall doesn't return.
    push    AF
    call    _status_clr_syscall
    pop     AF
    ei

__pexit_loop:
    jp      __pexit_loop

_sysinfo:
    .word   _kernel_version
__sysinfo_numbanks:
    .word   #0

    .globl  _kernel_version
_kernel_version:
    .asciz  "0.5.0"

__test:
    .asciz  "CANCEL handler: #0b04x\n\r"
