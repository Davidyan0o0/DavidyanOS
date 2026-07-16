#include "kernel/device/device.h"

#include "kernel/lib/string.h"

static DEVICE devices[DEVICE_MAX];
static unsigned int next_id = 1;

static void copy_field(char* dest,char* src)
{
    unsigned int i;

    for(i=0;i<DEVICE_NAME_MAX-1 && src[i];i++)
    {
        dest[i] = src[i];
    }

    dest[i] = 0;
}

void device_init()
{
    unsigned int i;

    memset(devices,0,sizeof(devices));
    next_id = 1;

    for(i=0;i<DEVICE_MAX;i++)
    {
        devices[i].present = 0;
    }

    device_register("vga0","video");
    device_register("kbd0","input");
    device_register("pit0","timer");
    device_register("rtc0","clock");
}

int device_register(char* name,char* type)
{
    unsigned int i;

    for(i=0;i<DEVICE_MAX;i++)
    {
        if(!devices[i].present)
        {
            devices[i].present = 1;
            devices[i].id = next_id;
            next_id++;
            copy_field(devices[i].name,name);
            copy_field(devices[i].type,type);
            return 1;
        }
    }

    return 0;
}

DEVICE* device_get(unsigned int index)
{
    unsigned int i;
    unsigned int seen = 0;

    for(i=0;i<DEVICE_MAX;i++)
    {
        if(devices[i].present)
        {
            if(seen==index)
            {
                return &devices[i];
            }

            seen++;
        }
    }

    return 0;
}

unsigned int device_count()
{
    unsigned int i;
    unsigned int count = 0;

    for(i=0;i<DEVICE_MAX;i++)
    {
        if(devices[i].present)
        {
            count++;
        }
    }

    return count;
}
