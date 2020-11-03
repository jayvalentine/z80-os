/* disk_drivers.h */
/* Signatures for disk driver functions defined in disk_drivers.asm. */

void init_disk();
void read_sector(char * buf, unsigned long sector);
void write_sector(char * buf, unsigned long sector);
