/* Commonly used macros etc. for bit manipulation. */
#ifndef _BITS_H
#define _BITS_H

#define BIT_IS_SET(_var, _bit) (((_var) & (_bit)) == (_bit))
#define BIT_IS_CLR(_var, _bit) (((_var) & (_bit)) == 0)

#define BIT_SET(_var, _bit) ((_var) |= (_bit))
#define BIT_CLR(_var, _bit) ((_var) &= ~(_bit))

#endif /* _BITS_H */
