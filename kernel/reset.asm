    ; Reset vectors for Z80-OS kernel.

    .equ    UART_PORT_DATA, #0b00000001
    .equ    UART_PORT_CONTROL, #0b00000000

    .globl  _main
    .globl  _break_handler
    .globl  _syscall_handler
    .globl  _interrupt_handler
    .globl  _disk_init

    .area   RESET   (ABS)

    ; Cold start entry point.
    .org    0x0000
_reset:
    ; Disable interrupts on startup.
    di
    jp      _cold_start

    ; Warm start entry point.
    .org    0x0008
_reset_warm:
    di
    jp      _start

    ; at 0x0028
    .org    0x0028
_break_entry:
    jp      _break_handler

    ; at 0x0030
    .org    0x0030
_syscall_entry:
    jp      _syscall_handler

    ; at 0x0038
    .org    0x0038
_interrupt_entry:
    jp      _interrupt_handler

    .globl  _startup_flags

_cold_start:
    ; Set startup flags, because we're cold-starting.
    ld      A, #0x00
    ld      (_startup_flags), A

_start:
    ; Initialise stack.
    ld      SP, #0x7fff

    ld      A, #0b11111101
    out     (0x80), A

    ; Initialize UART.

    ; Master reset.
    ld      A, #0b00000011
    out     (UART_PORT_CONTROL), A

    ; Configure UART.
    ; UART will run at #57600baud with a #3.6864MHz clock.
    ; Word length of #8 bits + #1 stop.
    ; Interrupts enabled on RX, disabled on TX.
    ld      A, #0b10010110
    out     (UART_PORT_CONTROL), A
    
    call    _disk_init

    ; Interrupt mode #1.
    im      #1

    ld      A, #0b11111111
    out     (0x80), A
    
    call    _main

    ; We should never return from main. If so, set an LED and halt.
    ld      A, #0b01111111
    out     (0x80), A
    halt
