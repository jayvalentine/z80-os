    ; Definitions
    .equ    UART_PORT_DATA, 0b00000001
    .equ    UART_PORT_CONTROL, 0b00000000

    ; void interrupt_init(void)
    ;
    ; Initializes interrupt system.
    .globl  _interrupt_init
_interrupt_init:
    ld      A, #0
    ld      (__interrupt_timer_ticks), A
    ret


    
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
    jp      z, #__interrupt_skip_6850
    
    ; Character received?
    bit     #0, A
    jp      nz, #__serial_read_handler

    ; Not a #6850 interrupt.
    ; Assume this is a timer interrupt as CURRENTLY
    ; we do not have any other interrupt sources.
__interrupt_skip_6850:
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
    .globl  _signal_get_handler
    .globl  _status_is_set_kernel
    .globl  _timer_reset

    ; These two symbols need to be global for benchmarking.
    .globl  __timer_handler
    .globl  __timer_handler_end

__interrupt_timer_ticks:
    .byte   0

__timer_handler:
    ; Increment tick count.
    ld      A, (__interrupt_timer_ticks)
    inc     A
    ld      (__interrupt_timer_ticks), A

    ; Skip timer handler if we are currently executing in kernel space.
    call    _status_is_set_kernel
    cp      #0
    jp      nz, __timer_handler_end

    ; Check tick count.
    ; We only context-switch a minimum of every 6 ticks
    ; so that we don't overload the processor.
    ld      A, (__interrupt_timer_ticks)
    cp      #6
    jp      c, __timer_handler_end

    ; Reset tick count.
    ld      A, #0
    ld      (__interrupt_timer_ticks), A



    ; CONTEXT SWITCHING

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

    ; Set stack to kernel space.
    ld      SP, #0x7ffe

    ; Call the scheduler to allocate another process.
    ; New RAM bank is returned in A.
    call    _scheduler_tick
    call    _ram_bank_set

    ; Check if there are any signals to handle for the
    ; current process.
    ;
    ; Will return handler address in DE (0 if no signal).
    call    _signal_get_handler
    ld      A, D
    or      E

    jp      z, __timer_handler_ret_to_process

    ; Return to process, calling signal handler.
    ; DE holds signal handler address.
__timer_handler_ret_to_handler:
    ld      SP, (0xfffe)

    ; Return address at SP+12
    ; Overwrite with handler address.
    ; Save original return address in BC.
    ld      HL, #12
    add     HL, SP
    ld      C, (HL)
    ld      (HL), E
    inc     HL
    ld      B, (HL)
    ld      (HL), D

    ; Ensure HL holds original return address.
    push    BC
    pop     HL



    ; Restore all registers aside from HL.
    pop     IY
    pop     IX
    pop     BC
    pop     DE
    inc     SP
    inc     SP
    pop     AF

    ex      AF, AF'
    exx

    jp      __timer_handler_end



    ; Return to process with no signal handler.
__timer_handler_ret_to_process:
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
    ; Reset the timer (clears the interrupt).
    call    _timer_reset

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
