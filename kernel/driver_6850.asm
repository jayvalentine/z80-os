    ; 6850 ACIA Drivers.

    defc    UART_PORT_DATA = 0b00000001
    defc    UART_PORT_CONTROL = 0b00000000

    PUBLIC  _driver_6850_tx

    ; driver_6850_tx(char * s, size_t count);
    PUBLIC  _driver_6850_tx
_driver_6850_tx:
    ld      HL, 2
    add     HL, SP

    ; Get count into BC, s into DE.
    ld      C, (HL)
    inc     HL
    ld      B, (HL)
    inc     HL

    ld      E, (HL)
    inc     HL
    ld      D, (HL)

_driver_6850_tx_loop:
    ; Loop until ready to transmit.
    in      A, (UART_PORT_CONTROL)
    bit     1, A
    jp      z, _driver_6850_tx_loop

    ; Ready to transmit now!
    ld      A, (DE)
    out     (UART_PORT_DATA), A

    inc     DE
    dec     BC

    ; If BC is 0, exit.
    ; Otherwise, continue loop.
    ld      A, B
    or      C
    jp      nz, _driver_6850_tx_loop

    ret
