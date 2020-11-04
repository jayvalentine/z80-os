    ; Reset vectors for Z80-OS kernel.

    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000

    EXTERN  _main
    EXTERN  _syscall_handler
    EXTERN  _interrupt_handler
    EXTERN  _disk_init
    
    ; Entry point.
_reset:
    ; Disable interrupts on startup.
    di
    jp      _start

    defs    0x0030 - ASMPC
_syscall_entry:
    jp      _syscall_handler

    defs    0x0038 - ASMPC
_interrupt_entry:
    jp      _interrupt_handler

_start:
    ; Initialise stack.
    ld      SP, $ffff

    ld      A, 0b11111101
    out     ($80), A

    ; Initialize UART.

    ; Master reset.
    ld      A, 0b00000011
    out     (UART_PORT_CONTROL), A

    ; Configure UART.
    ; UART will run at 57600baud with a 3.6864MHz clock.
    ; Word length of 8 bits + 1 stop.
    ; Interrupts enabled on RX, initially disabled on TX.
    ld      A, 0b10010110
    out     (UART_PORT_CONTROL), A
    
    call    _disk_init

    ; Enable interrupts, mode 1.
    im      1
    ei

    jp      _main
