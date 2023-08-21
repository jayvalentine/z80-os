#include "t_defs.h"

#define ARRAY_SIZE(_arr) *((const tok_t *)_arr + 1)
#define ARRAY_ACCESS(_arr, _index) ((numeric_t *)(_arr + 2))[_index-1]
