#include "defs.h"

void kernel(void);

#ifdef Z88DK
void set_reg(ubyte val) __z88dk_fastcall;
#else
void set_reg(ubyte val);
#endif

bool ram_test(void);
