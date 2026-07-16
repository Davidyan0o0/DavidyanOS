#ifndef RAMDISK_H
#define RAMDISK_H

#define RAMDISK_SECTOR_SIZE 512
#define RAMDISK_SECTOR_COUNT 32

void ramdisk_init();
unsigned int ramdisk_sector_count();
unsigned int ramdisk_size_bytes();
int ramdisk_read(unsigned int sector,char* out);
int ramdisk_write(unsigned int sector,char* data);
void ramdisk_fill_demo();

#endif
