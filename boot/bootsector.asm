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
    jr      _code
    defs    $003e-$0002
_code:
    ld      DE, $8000+_message-_bootsector
_code_loop:
    ld      A, (DE)
    inc     DE

    ; Done if we hit null.
    cp      0
    jr      z, _code_done

    ; Print character.
    ld      L, A
    ld      A, 0
    rst     48

    ; Loop.
    jr      _code_loop

_code_done:
    ret

_message:
    defm    "Booting Z80-OS...\n\r"
    defb    0
