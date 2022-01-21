#ifndef _SERIAL_H
#define _SERIAL_H

/* Routines for serial handling. */

/* serial_init
 *
 * Initializes serial drivers.
 */
void serial_init(void);

/* serial_tx_pop
 *
 * Purpose:
 *     Get the next character to transmit
 *     from the tx buffer.
 * 
 * Parameters:
 *     None.
 * 
 * Returns:
 *     Character to transmit.
 */
char serial_tx_pop(void);

/* serial_tx_push
 *
 * Purpose:
 *     Add a character to transmit
 *     to the tx buffer.
 * 
 * NOTE: Blocks until there is space
 * in buffer to place character.
 * 
 * Parameters:
 *     Character to transmit.
 * 
 * Returns:
 *     Nothing.
 */
void serial_tx_push(char c);

#endif
