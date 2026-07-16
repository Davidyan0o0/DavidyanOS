#ifndef RAMFS_H
#define RAMFS_H

#define RAMFS_NAME_MAX 24
#define RAMFS_FILE_MAX 16
#define RAMFS_DATA_MAX 256

typedef struct
{
    char name[RAMFS_NAME_MAX];
    unsigned int size;
    unsigned int used;
    unsigned int directory;
    char data[RAMFS_DATA_MAX];
} RAMFS_NODE;

void ramfs_init();
int ramfs_create(char* name,char* data);
int ramfs_mkdir(char* name);
int ramfs_touch(char* name);
int ramfs_write(char* name,char* data);
int ramfs_append(char* name,char* data);
int ramfs_rename(char* old_name,char* new_name);
int ramfs_delete(char* name);
RAMFS_NODE* ramfs_find(char* name);
RAMFS_NODE* ramfs_get(unsigned int index);
unsigned int ramfs_dir_count();
unsigned int ramfs_count();
unsigned int ramfs_capacity();
unsigned int ramfs_used_bytes();
unsigned int ramfs_free_bytes();

#endif
