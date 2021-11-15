    PUBLIC  _status_set

    ; void status_set(uint8_t val) __z88dk_fastcall
_status_set:
    ld      A, L
    or      %00000001 ; Make sure LOW-RAM is always enabled.
    out     ($80), A
    ret
