.globl  _main

    ; no-ops - we're just testing that we hit main.
_main:
    nop
    nop
    nop
    nop

    ; Linker fails without this data definition
    .area   _DATA (REL)
_data:
    nop
