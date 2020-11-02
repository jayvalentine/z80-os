;    Boot sector image for Z80-OS.
;    Copyright (C) 2020 Jay Valentine
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <https://www.gnu.org/licenses/>.

_bootsector:
    jr      _boot_message
    defs    $003e-$0002

_boot_message:
    ld      DE, $8000+_message_booting-_bootsector
    call    $8000+_print-_bootsector

_boot_message_done:

    ; Now load the 15 reserved sections into $8200.
    ld      DE, 0
    ld      BC, 1
    ld      HL, $8200

_boot_load_loop:
    push    DE
    push    BC
    push    HL

    ; dread syscall
    ld      A, 3 << 1
    rst     48

    pop     HL
    pop     BC
    pop     DE

    ; Next sector
    inc     BC

    ; Next 512 bytes of memory
    ld      DE, 512
    adc     HL, DE
    ld      DE, 0

    ; Stop once we hit sector 16
    ld      A, C
    cp      16
    jr      nz, _boot_load_loop

_boot_load_done:
    ; Done, so call the code we've just loaded.
    ld      DE, $8000+_message_done-_bootsector
    call    $8000+_print-_bootsector

    call    $8200

_print:
    ld      A, (DE)
    inc     DE

    ; Done if we hit null.
    cp      0
    jr      z, _print_done

    ; Print character.
    ld      L, A
    ld      A, 0
    rst     48

    ; Loop.
    jr      _print

_print_done:
    ret

_message_booting:
    defm    "Booting Z80-OS... "
    defb    0

_message_done:
    defm    "Done.\n\r"
    defb    0
