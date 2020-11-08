#ifndef _CHMOD_H
#define _CHMOD_H

#include <stddef.h>

#define FATTR_SYS 0b00000100
#define FATTR_HID 0b00000010
#define FATTR_RO  0b00000001

int command_chmod(char ** argv, size_t argc);

#endif /* _CHMOD_H */
