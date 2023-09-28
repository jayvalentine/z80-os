    ; Definitions
    .equ    UART_PORT_DATA, 0b00000001
    .equ    UART_PORT_CONTROL, 0b00000000

    .equ    TIMER_CONTROL, 0x10
    
    .globl  _interrupt_handler

    .globl  _status_set_int
    .globl  _status_clr_int

_interrupt_handler:
    di
    exx
    ex      AF, AF'

    call    _status_set_int

    ; Get return address (TOS) into HL.
    pop     HL
    push    HL ; Don't forget to restore it!

    ; Is this an interrupt from the #6850?
    in      A, (UART_PORT_CONTROL)
    bit     #7, A
    jp      z, #__interrupt_skip2
    
    ; Character received?
    bit     #0, A
    jp      nz, #__serial_read_handler

    ; Not a #6850 interrupt.
__interrupt_skip2:
    ; Is this a timer interrupt?
    in      A, (TIMER_CONTROL)
    cp      #1
    jp      nz, #__interrupt_skip3

    jp      __timer_handler

__interrupt_skip3:
    ; Not any of the known causes.
    jp      __unknown_interrupt

__interrupt_handle_ret:
    call    _status_clr_int
    
    ex      AF, AF'
    exx
    ei

    .globl  __interrupt_handler_end ; Required for benchmarking.
__interrupt_handler_end:
    reti



    ; void terminal_put(char c)
    .globl  _terminal_put

    ; Needed for tests...
    ; TODO: Find a way around this.
    .globl  __serial_read_handler

__serial_read_handler:
    ; Read data from UART and send to terminal.
    in      A, (UART_PORT_DATA)
    call    _terminal_put
    
    jp      __interrupt_handle_ret



    .globl  _scheduler_tick
    .globl  _ram_bank_set

    ; These two symbols need to be global for benchmarking.
    .globl  __timer_handler
    .globl  __timer_handler_end
__timer_handler:
    ; Switch to user register set and stack all registers.
    exx
    ex      AF, AF'
    push    AF
    push    HL
    push    DE
    push    BC
    push    IX
    push    IY

    ld      (0xfffe), SP

    ; Call the scheduler to allocate another process.
    ; New RAM bank is returned in A.
    call    _scheduler_tick
    call    _ram_bank_set

    ld      SP, (0xfffe)

    ; Now unstack all registers and switch back
    ; to system register set.
    pop     IY
    pop     IX
    pop     BC
    pop     DE
    pop     HL
    pop     AF
    ex      AF, AF'
    exx

    ; Return from the interrupt.
__timer_handler_end:
    jp      __interrupt_handle_ret

__unknown_interrupt:
    jp      __interrupt_handle_ret

    ; void interrupt_enable(void)
    .globl  _interrupt_enable
_interrupt_enable:
    ei
    ret

    ; void interrupt_disable(void)
    .globl  _interrupt_disable
_interrupt_disable:
    di
    ret

    ; void halt(void)
    .globl _halt
_halt:
    halt

    ; void interrupt_tx_enable(void)
    .globl  _interrupt_tx_enable
_interrupt_tx_enable:
    ; Enable TX interrupts
    ld      A, #0b10110110
    out     (UART_PORT_CONTROL), A
    ret

    ; void interrupt_tx_disable(void)
    .globl  _interrupt_tx_disable
_interrupt_tx_disable:
    ld      A, #0b10010110
    out     (UART_PORT_CONTROL), A
    ret
