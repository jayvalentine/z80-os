    ; Reset vectors for Z80-OS kernel.

    EXTERN  _main

_reset:
    jp      _main

    ; Lots of padding, to really exercise the filesystem.
    defs    $1234
