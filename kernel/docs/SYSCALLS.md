# System Calls

System calls provided by the ZEBRA OS kernel are described here.

System calls are implemented via a handler at location `0x0030` in memory.
The desired syscall is selected by passing an ID in the `A` register.

The ID is used as an index into a function pointer table to execute the
correct system call implementation. Therefore each ID must be a multiple
of two bytes.

The handler assumes that syscall parameters are passed according to the SDCC
version 1 calling convention.

## List of System Calls

### Terminal Interaction

#### 0: `void swrite(const char * s, size_t count)`

Writes `count` bytes pointed to by `s` to the terminal.

#### 2: `int sread(void)`

Returns a byte received from the terminal, zero-extended to 16 bits.
If no byte is available, returns `-1`.
