#ifndef DISKFS_H
#define DISKFS_H

#define DISKFS_MAX_FILES 8
#define DISKFS_NAME_MAX 24
#define DISKFS_DATA_MAX 512

#define DISKFS_ERROR_NONE 0
#define DISKFS_ERROR_NOT_MOUNTED 1
#define DISKFS_ERROR_BAD_NAME 2
#define DISKFS_ERROR_NO_SPACE 3
#define DISKFS_ERROR_DATA_WRITE 4
#define DISKFS_ERROR_SUPER_WRITE 5
#define DISKFS_ERROR_DATA_READ 6

typedef struct
{
    char name[DISKFS_NAME_MAX];
    unsigned int size;
    unsigned int used;
} DISKFS_ENTRY;

void diskfs_init();
int diskfs_format();
int diskfs_mount();
int diskfs_write(char* name,char* data);
int diskfs_read(char* name,char* out,unsigned int capacity);
int diskfs_delete(char* name);
DISKFS_ENTRY* diskfs_get(unsigned int index);
unsigned int diskfs_count();
unsigned int diskfs_mounted();
unsigned int diskfs_last_error();

#endif
