    ; 8254 Programmable Timer drivers.
    .equ    TIMER_0, 0x10
    .equ    TIMER_CTRL, 0x13

    ; At 3.686400 MHz each cycle is ~0.27us.
    ; 60,000 cycles is ~16.28ms.
    .equ    TIMER_COUNT, 60000

    ; void timer_init(void)
    ;
    ; Initializes timer 0 as recurring interrupt timer.
    .globl _timer_init
_timer_init:
    ; format is:
    ;     sc:rw:mmm:b
    ;
    ; sc = 0  (select timer 0)
    ; rw = 3  (r/w both bytes)
    ; m = 0   (mode 0)
    ; b = 0   (binary counter)
    ;
    ld      A, #0b00110000
    out     (TIMER_CTRL), A

    call    _timer_reset

    ret

    ; void timer_reset(void)
    ;
    ; Resets timer 0 to initial count.
    .globl _timer_reset
_timer_reset:
    ld      C, #TIMER_0
    ld      HL, #TIMER_COUNT
    out     (C), L ; lsb
    out     (C), H ; msb
    ret
