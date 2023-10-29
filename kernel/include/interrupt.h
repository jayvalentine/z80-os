#ifndef _INTERRUPT_H
#define _INTERRUPT_H

void interrupt_init(void);

void interrupt_enable(void);
void interrupt_disable(void);
void interrupt_tx_enable(void);
void interrupt_tx_disable(void);

#endif
