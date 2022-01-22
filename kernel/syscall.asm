    ; Definitions
    .equ    UART_PORT_DATA, #0b00000001
    .equ    UART_PORT_CONTROL, #0b00000000

    .globl  _driver_6850_tx
    .globl  _serial_mode

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

    .globl  _signal_sethandler

    ; Syscall table.
_syscall_table:
    .word   _driver_6850_tx         ; swrite
    .word   _do_sread               ; sread

    ; DREAD, #DWRITE no longer supported.
    .word   __invalid_syscall
    .word   __invalid_syscall

    .word   _file_open              ; fopen
    .word   _file_read              ; fread
    .word   _file_write             ; fwrite
    .word   _file_close             ; fclose

    .word   _do_dinfo               ; dinfo
    .word   _file_info              ; finfo

    .word   _file_entries           ; fentries
    .word   _file_entry             ; fentry

    .word   _process_spawn          ; pspawn

    .word   _signal_sethandler      ; sighandle

    .word   _file_delete            ; fdelete
    
    .word   _process_load           ; pload

    .word   _serial_mode            ; smode

    .word   _do_sysinfo             ; sysinfo

    .word   _scheduler_state        ; pstate
    .word   _do_pexit               ; pexit
    .word   _scheduler_exitcode     ; pexitcode

    .globl  _syscall_handler

    .globl  _status_set_syscall
    .globl  _status_clr_syscall

    ; Syscall handler.
    ;
    ; Calculates absolute position of syscall function address
    ; using offset provided in A, then executes the function at
    ; that address.
_syscall_handler:
    di

    ; Save IX, HL, DE, BC
    push    IX
    push    HL
    push    DE
    push    BC

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
    pop     IX

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

    .globl  _startup_flags

    ; Executed when we see an invalid syscall.
    ; Performs a warm restart of the kernel.
__invalid_syscall:
    ld      A, #0x01
    ld      (_startup_flags), A
    rst     8

    ; #1: sread: Read character from serial port.
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
    jp      z, #__sread_wait
    di

__sread_available:
    ; Calculate offset into buffer.
    ld      HL, #_rx_buf
    ld      D, #0
    add     HL, DE

    ; Load character into A.
    ld      A, (HL)

    ; Increment head.
    ld      HL, #_rx_buf_offs_head
    inc     (HL)

    pop     HL
    pop     DE
    ret

    .globl  _rx_buf_offs_head
    .globl  _rx_buf_offs_tail
    .globl  _rx_buf

_rx_buf_offs_head:
    .byte   #0
_rx_buf_offs_tail:
    .byte   #0

_rx_buf:
    .ds     #256

    .globl  _disk_info

    ; #8: dinfo: Get information about disk.
    ;
    ; Parameters: None
    ;
    ; Returns:
    ; Pointer to disk info struct, in HL.
_do_dinfo:
    ld      HL, #_disk_info
    ret

    ; #17: sysinfo: Get info about kernel.
    ;
    ; Parameters:
    ; None.
    ;
    ; Returns:
    ; Pointer to sysinfo struct in HL.
_do_sysinfo:
    ld      HL, #_sysinfo
    ret

    .globl  _sysinfo

    ; #19: pexit: Exit current process.
    ;
    ; Parameters:
    ; BC: exit code.
    ;
    ; Returns:
    ; Nothing.
_do_pexit:
    ld      HL, #2
    add     HL, SP
    ld      C, (HL)
    inc     HL
    ld      B, (HL)

    push    BC
    
    call    _process_exit

    ; This syscall doesn't return.
    ; Set return address to the loop function
    ; and return.
    ld      HL, #__pexit_loop
    push    HL
    ld      HL, #__syscall_ret
    push    HL

    ret

__pexit_loop:
    jp      __pexit_loop

_sysinfo:
    .word   __kernel_version
__sysinfo_numbanks:
    .word   #0

__kernel_version:
    .asciz  "0.4.0"

__test:
    .asciz  "CANCEL handler: #0b04x\n\r"
