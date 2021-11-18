#ifndef _STATUS_H
#define _STATUS_H

/* Initialize status. */
void status_init(void);

/* Set interrupt status. */
void status_set_int(void);

/* Clear interrupt status. */
void status_clr_int(void);

/* Set syscall status. */
void status_set_syscall(void);

/* Clear syscall status. */
void status_clr_syscall(void);

#endif
