    ; #6850 ACIA Drivers.

    .equ    UART_PORT_DATA, 0b00000001
    .equ    UART_PORT_CONTROL, 0b00000000

    .globl  _driver_6850_tx

    ; driver_6850_tx(char * s, size_t count)
    ;
    ; s     will be in HL
    ; count will be in DE
    ;
    .globl  _driver_6850_tx
    .globl  _driver_6850_tx_done ; Required for benchmarking.
_driver_6850_tx:
    push    IX

_driver_6850_tx_loop:
    ; Loop until ready to transmit.
    in      A, (UART_PORT_CONTROL)
    bit     #1, A
    jp      z, _driver_6850_tx_loop

    ; Ready to transmit now!
    ld      A, (HL)
    out     (UART_PORT_DATA), A

    inc     HL
    dec     DE

    ; If BC is #0, exit.
    ; Otherwise, continue loop.
    ld      A, D
    or      E
    jp      nz, _driver_6850_tx_loop

_driver_6850_tx_done:
    pop     IX
    ret
