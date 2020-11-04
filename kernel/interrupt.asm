    ; Definitions
    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000
    
    PUBLIC  _interrupt_handler

_interrupt_handler:
    di
    push    HL
    push    DE
    push    BC
    push    AF

    ; Is this an interrupt from the 6850?
    in      A, (UART_PORT_CONTROL)
    bit     7, A
    jp      z, __interrupt_skip2
    
    ; Character received?
    bit     0, A
    jp      z, __interrupt_skip1

    call    _serial_read_handler
    jp      __interrupt_handle_ret
__interrupt_skip1:

    ; Ready to transmit character?
    bit     1, A
    jp      z, __interrupt_skip2

    call    _serial_write_handler
    jp      __interrupt_handle_ret
__interrupt_skip2:

    ; Not any of the known causes.
    call    _unknown_interrupt

__interrupt_handle_ret:
    pop     AF
    pop     BC
    pop     DE
    pop     HL
    ei
    reti

    EXTERN  _rx_buf
    EXTERN  _rx_buf_offs_head
    EXTERN  _rx_buf_offs_tail

_serial_read_handler:
    ; Get current tail of buffer.
    ld      HL, _rx_buf
    ld      D, 0
    ld      A, (_rx_buf_offs_tail)
    ld      E, A
    adc     HL, DE

    ; Read data from UART.
    in      A, (UART_PORT_DATA)

    ; Store received character.
    ld      (HL), A

    ; Increment tail.
    ld      HL, _rx_buf_offs_tail
    inc     (HL)

    ret

    EXTERN  _tx_buf
    EXTERN  _tx_buf_offs_head
    EXTERN  _tx_buf_offs_tail

_serial_write_handler:
    ; Get current head and tail of buffer.
    ld      A, (_tx_buf_offs_tail)
    ld      E, A

    ld      A, (_tx_buf_offs_head)

    ; If equal, we've got nothing to transmit.
    ; In this case, we disable tx interrupts and return.
    cp      E
    jp      nz, __tx

    ld      A, 0b10010110
    out     (UART_PORT_CONTROL), A
    ret
__tx:

    ; Otherwise, we've got something to send.
    ; Calculate buffer offset.
    ld      HL, _tx_buf
    ld      D, 0
    adc     HL, DE

    ; Load character and send.
    ld      A, (HL)
    out     (UART_PORT_DATA), A

    ; Increment tail.
    ld      HL, _tx_buf_offs_tail
    inc     (HL)

    ret

_unknown_interrupt:
    ret
