# Process Handling

Each process in the ZEBRA kernel is allocated a 32KB "page" of the banked user-RAM.
A process's page is structured as below:

* `0x8000`-`0xdfff`: Code and data space
* `0xe000`-`0xf7ff`: Stack
* `0xf800`-`0xffff`: System per-process storage (argv, argc, state on context switch)

Active processes are referenced by the process table, 

## Signals

Each process table entry maintains a set of flags indicating the signals that have been triggered
for a process.

When a process is scheduled, the signal flags are checked in priority order.
Each signal for which the flag is set is handled. If a signal causes the process
to exit, then no further signals are handled.

### Signal Handling

Each signal has a default handler set by the kernel. Some signals can have a handler
set by the process which will be called instead.
