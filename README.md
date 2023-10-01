# ZEBRA Z80 Operating System

ZEBRA is a multi-tasking operating system for Z80 computers, implemented mostly in C with some assembler.

## License

This software is licensed under GPLv3. For the full license terms, see [here](LICENSE).

## Contributing

If you are interested in getting the OS set up for your own system please get in touch!

## Overview

The operating system targets my custom Z80 modular computer.

The kernel is intended to run with 32KB of RAM and provides user interaction via a serial interface.
User programs are each allocated a 32KB page of banked RAM, allowing the multiple programs to be executing
concurrently.

This repository also contains some builtin utility programs, such as a BASIC
interpreter.

### Current Functionality

#### Bootloader

Two-stage bootloader allowing loading of OS images from filesystem

#### Kernel

* System:
  * Multi-tasking using banked RAM
  * Syscalls for hardware abstraction, implemented using Z80 `rst` instruction
  * FAT16 "flat" filesystem (no directory handling)
    supporting the following operations:
    * Writing new files (no "append" mode)
    * Reading files
    * Deleting files
* Hardware:
  * TTL serial I/O
  * CompactFlash card for data storage

#### Command Processor

* Simple command-line interface
* Built-in commands, e.g. `dir` for viewing files
* Executable loader for loading programs from disk

### Planned Functionality (Short-Term)

* Debugger/monitor
* Loading programs over serial in Intel-HEX format
* Saving loaded programs to disk
* "Append" mode when writing to files

### Planned Functionality (Long-Term)

* Graphical interface (requires video hardware)

## System Design

### Boot Process

On startup the modular computer has 8k of ROM at `0x0000`, and 32k of RAM at `0x8000`.
A [bootloader](https://github.com/jayvalentine/z80-bootloader) resides in ROM and has basic
CF-card drivers able to read individual sectors from disk.

A secondary bootloader resides on the first 9 sectors of the CF-card. This second-stage loader
has the same CF-card routines but additionally has a FAT-16 driver able to read files from the disk,
which is formatted in FAT16 format.

This secondary bootloader is loaded into RAM at `0x8000` and then executed. It first switches the
lower bank (`0x0000` to `0x7fff`) from ROM to RAM and writes the kernel image, located on the CF-card's
filesystem as `kernel.bin`, to RAM at `0x0000`. The secondary loader then resets to `0x0000` to boot into
the kernel.

The kernel initializes a complete filesystem driver and other operating system components, then loads
the command-processor (`command.exe`) from the disk and begins executing it.

At this point control is handed to the user, who is able to interact with the system via the command-line
to run programs.

### Memory Map

~~~~
Low RAM                                  High RAM

0x0000┌────────────────────────┐         0x8000┌────────────────────────┐
      │ Kernel                 │               │ User Program Area      │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │               │                        │
      │                        │         0xe000├────────────────────────┤
      │                        │               │ Stack                  │
      │                        │               │                        │
      │                        │               │                        │
      │                        │         0xF800├────────────────────────┤
      │                        │               │ User Program Args      │
      └────────────────────────┘               └────────────────────────┘
~~~~

* Kernel: This is the memory area in which the kernel (including reset vectors) resides
* User Program Area: User programs are loaded into this memory area when executed
* Stack: Reserved for program stack. Stack pointer is initialized to `0xF7FF` on program startup and grows down.
* User Program Args: Reserved for user program arguments (`argv` and `argc`).
