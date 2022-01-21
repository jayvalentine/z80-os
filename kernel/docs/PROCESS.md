# Process Handling

Each process in the ZEBRA kernel is allocated a 32KB "page" of the banked user-RAM.
A process's page is structured as below:

* `0x8000`-`0xdfff`: Code and data space
* `0xe000`-`0xf7ff`: Stack
* `0xf800`-`0xffff`: System per-process storage (argv, argc, stack-pointer on context switch)
