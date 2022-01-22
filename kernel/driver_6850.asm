    ; #6850 ACIA Drivers.

    .equ    UART_PORT_DATA, 0b00000001
    .equ    UART_PORT_CONTROL, 0b00000000

    .globl  _driver_6850_tx

    ; driver_6850_tx(char * s, size_t count);
    .globl  _driver_6850_tx
    .globl  _driver_6850_tx_done ; Required for benchmarking.
_driver_6850_tx:
    push    IX

    ld      HL, #4
    add     HL, SP
    push    HL
    pop     IX

    ; Get count into BC, s into DE.
    ld      C, 2(IX)
    ld      B, 3(IX)

    ld      E, 0(IX)
    ld      D, 1(IX)

_driver_6850_tx_loop:
    ; Loop until ready to transmit.
    in      A, (UART_PORT_CONTROL)
    bit     #1, A
    jp      z, _driver_6850_tx_loop

    ; Ready to transmit now!
    ld      A, (DE)
    out     (UART_PORT_DATA), A

    inc     DE
    dec     BC

    ; If BC is #0, exit.
    ; Otherwise, continue loop.
    ld      A, B
    or      C
    jp      nz, _driver_6850_tx_loop

_driver_6850_tx_done:
    pop     IX
    ret
