#include "drivers/storage/ramdisk.h"

#include "kernel/lib/string.h"

static char sectors[RAMDISK_SECTOR_COUNT][RAMDISK_SECTOR_SIZE];

void ramdisk_init()
{
    memset(sectors,0,sizeof(sectors));
    ramdisk_fill_demo();
}

unsigned int ramdisk_sector_count()
{
    return RAMDISK_SECTOR_COUNT;
}

unsigned int ramdisk_size_bytes()
{
    return RAMDISK_SECTOR_COUNT*RAMDISK_SECTOR_SIZE;
}

int ramdisk_read(unsigned int sector,char* out)
{
    if(sector>=RAMDISK_SECTOR_COUNT)
    {
        return 0;
    }

    memcpy(out,sectors[sector],RAMDISK_SECTOR_SIZE);
    return 1;
}

int ramdisk_write(unsigned int sector,char* data)
{
    if(sector>=RAMDISK_SECTOR_COUNT)
    {
        return 0;
    }

    memset(sectors[sector],0,RAMDISK_SECTOR_SIZE);
    memcpy(sectors[sector],data,strlen(data));
    return 1;
}

void ramdisk_fill_demo()
{
    ramdisk_write(0,"DavidyanOS RAM disk sector zero");
    ramdisk_write(1,"Use disk read <n> and disk write <n> <text>");
}
