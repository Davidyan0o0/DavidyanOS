#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_NAME_MAX 16
#define DEVICE_MAX 16

typedef struct
{
    char name[DEVICE_NAME_MAX];
    char type[DEVICE_NAME_MAX];
    unsigned int id;
    unsigned int present;
} DEVICE;

void device_init();
int device_register(char* name,char* type);
DEVICE* device_get(unsigned int index);
unsigned int device_count();

#endif
