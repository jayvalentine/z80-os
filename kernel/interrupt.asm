    ; Definitions
    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000
    
    PUBLIC  _interrupt_handler

    EXTERN  _status_set_int
    EXTERN  _status_clr_int

_interrupt_handler:
    di
    exx
    ex      AF, AF'

    call    _status_set_int

    ; Get return address (TOS) into HL.
    pop     HL
    push    HL ; Don't forget to restore it!

    ; Is this an interrupt from the 6850?
    in      A, (UART_PORT_CONTROL)
    bit     7, A
    jp      z, __interrupt_skip2
    
    ; Character received?
    bit     0, A
    jp      nz, __serial_read_handler

    ; Ready to transmit character?
    bit     1, A
    jp      nz, __serial_write_handler

    ; Not a 6850 interrupt.
__interrupt_skip2:

    ; Not any of the known causes.
    jp      __unknown_interrupt

__interrupt_handle_ret:
    call    _status_clr_int
    
    ex      AF, AF'
    exx
    ei

__interrupt_handler_end:
    reti

    EXTERN  _rx_buf
    EXTERN  _rx_buf_offs_head
    EXTERN  _rx_buf_offs_tail

    EXTERN  _signal_cancel

    EXTERN  _serial_current_mode

__serial_read_handler:
    ; Save interrupt return address.
    push    HL

    ; Get current tail of buffer.
    ld      HL, _rx_buf
    ld      D, 0
    ld      A, (_rx_buf_offs_tail)
    ld      E, A
    add     HL, DE

    ; Read data from UART.
    in      A, (UART_PORT_DATA)
    
    ; $18 (CANCEL) - triggers SIG_CANCEL
    cp      $18
    jp      nz, __serial_read_byte

    ; Handle special characters only if in interactive mode.
    ld      C, A
    ld      A, (_serial_current_mode)
    cp      0
    jp      z, __serial_signal_cancel
    
    ; Need the byte, so restore A.
    ld      A, C

__serial_read_byte:
    ; Store received character.
    ld      (HL), A

    ; Increment tail.
    ld      HL, _rx_buf_offs_tail
    inc     (HL)

    pop     HL
    
    jp      __interrupt_handle_ret

__serial_signal_cancel:
    exx
    ei
    pop     HL
    call    _signal_cancel
    di
    exx
    
    jp      __interrupt_handle_ret

    EXTERN  _tx_buf
    EXTERN  _tx_buf_head
    EXTERN  _tx_buf_tail

__serial_write_handler:
    ; Get current head and tail of buffer.
    ld      HL, (_tx_buf_head)
    ex      DE, HL
    ld      HL, (_tx_buf_tail)

    ; Head in DE, tail in HL.
    ; Compare to check if they're equal.
    ; Only need to compare lower half
    ; due to alignment.
    ld      A, L
    cp      E
    jp      nz, __tx

    ; If equal, we've got nothing to transmit.
    ; In this case, we disable tx interrupts and return.
    ld      A, 0b10010110
    out     (UART_PORT_CONTROL), A
    
    jp      __interrupt_handle_ret

__tx:
    ; Load character and send.
    ld      A, (HL)
    out     (UART_PORT_DATA), A

    ; Increment tail.
    inc     L
    ld      (_tx_buf_tail), HL

    jp      __interrupt_handle_ret

__unknown_interrupt:
    jp      __interrupt_handle_ret
