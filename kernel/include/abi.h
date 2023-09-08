#ifndef _ABI_H
#define _ABI_H

/* STACKCALL is a calling convention that passes parameters on the stack. */
#ifdef Z80
#define STACKCALL __sdcccall(0)
#else
#define STACKCALL
#endif

#endif
