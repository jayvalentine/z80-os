# Terminal

Each process maintains its own terminal. At most one terminal can be set as the
_active_ terminal, which sends and receives data via the user serial port.

Each terminal has the following attributes:

* `status`: Set of status bit-flags for the terminal:
  * `TERM_ISACTIVE`: Set if this terminal is the active terminal.
  * `TERM_AVAILABLE`: Set if this terminal has received bytes that the process
    can read. This flag will only be set when `TERM_ISACTIVE` is also set.
  * `TERM_INTERACTIVE`: Set if this terminal is in _interactive mode_.
    In interactive mode, certain bytes from the serial port are treated as signals from the user.
    In non-interactive mode, all bytes are treated as raw data (e.g. used for file transfer).

Due to memory constraints, terminal data is not buffered separately for each process.
Instead, the process with the active terminal may read and write data from the I/O device.

All other processes will block until `TERM_AVAILABLE` is set.

## User Signals

In interactive mode, the following bytes result in signals for the process
with the active terminal:

* `0x18` (cancel): `SIG_CANCEL`
