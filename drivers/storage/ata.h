#ifndef ATA_H
#define ATA_H

#define ATA_SECTOR_SIZE 512

void ata_init();
int ata_identify();
int ata_read_sector(unsigned int lba,unsigned char* buffer);
int ata_write_sector(unsigned int lba,unsigned char* buffer);
unsigned int ata_last_status();

#endif
