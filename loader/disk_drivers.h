#ifndef _DISK_DRIVERS_H

/* disk_drivers.h */
/* Signatures for disk driver functions defined in disk_drivers.asm. */

void disk_init();
void read_sector(char * buf, unsigned long sector);
void write_sector(char * buf, unsigned long sector);

#endif /* _DISK_DRIVERS_H */
