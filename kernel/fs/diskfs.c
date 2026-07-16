#include "kernel/fs/diskfs.h"

#include "drivers/storage/ata.h"
#include "kernel/lib/string.h"

#define DISKFS_BASE_LBA 256
#define DISKFS_MAGIC 0x44534631

typedef struct
{
    unsigned int magic;
    unsigned int version;
    DISKFS_ENTRY entries[DISKFS_MAX_FILES];
} DISKFS_SUPER;

static DISKFS_SUPER super;
static unsigned int mounted = 0;
static unsigned int last_error = DISKFS_ERROR_NONE;

static void clear_sector(unsigned char* sector)
{
    memset(sector,0,512);
}

static void copy_name(char* dest,char* src)
{
    unsigned int i;

    for(i=0;i<DISKFS_NAME_MAX-1 && src[i];i++)
    {
        dest[i] = src[i];
    }

    dest[i] = 0;
}

static int flush_super()
{
    unsigned char sector[512];
    clear_sector(sector);
    memcpy(sector,&super,sizeof(DISKFS_SUPER));
    if(!ata_write_sector(DISKFS_BASE_LBA,sector))
    {
        last_error = DISKFS_ERROR_SUPER_WRITE;
        return 0;
    }
    return 1;
}

static int load_super()
{
    unsigned char sector[512];
    if(!ata_read_sector(DISKFS_BASE_LBA,sector))
    {
        return 0;
    }
    memcpy(&super,sector,sizeof(DISKFS_SUPER));
    return super.magic==DISKFS_MAGIC;
}

void diskfs_init()
{
    if(!diskfs_mount())
    {
        diskfs_format();
        diskfs_mount();
    }
}

int diskfs_format()
{
    unsigned int i;
    unsigned char sector[512];
    int ok = 1;

    last_error = DISKFS_ERROR_NONE;
    memset(&super,0,sizeof(DISKFS_SUPER));
    super.magic = DISKFS_MAGIC;
    super.version = 1;
    for(i=0;i<DISKFS_MAX_FILES;i++)
    {
        super.entries[i].used = 0;
        clear_sector(sector);
        if(!ata_write_sector(DISKFS_BASE_LBA+1+i,sector))
        {
            ok = 0;
            last_error = DISKFS_ERROR_DATA_WRITE;
        }
    }
    mounted = ok && flush_super();
    return mounted;
}

int diskfs_mount()
{
    mounted = load_super();
    return mounted;
}

static int find_file(char* name)
{
    unsigned int i;

    for(i=0;i<DISKFS_MAX_FILES;i++)
    {
        if(super.entries[i].used && strcmp(super.entries[i].name,name)==0)
        {
            return (int)i;
        }
    }

    return -1;
}

static int find_free()
{
    unsigned int i;

    for(i=0;i<DISKFS_MAX_FILES;i++)
    {
        if(!super.entries[i].used)
        {
            return (int)i;
        }
    }

    return -1;
}

int diskfs_write(char* name,char* data)
{
    int index;
    unsigned char sector[512];
    unsigned int size;

    last_error = DISKFS_ERROR_NONE;
    if(!mounted)
    {
        last_error = DISKFS_ERROR_NOT_MOUNTED;
        return 0;
    }
    if(name[0]==0)
    {
        last_error = DISKFS_ERROR_BAD_NAME;
        return 0;
    }

    index = find_file(name);
    if(index<0)
    {
        index = find_free();
    }
    if(index<0)
    {
        last_error = DISKFS_ERROR_NO_SPACE;
        return 0;
    }

    clear_sector(sector);
    size = strlen(data);
    if(size>DISKFS_DATA_MAX-1)
    {
        size = DISKFS_DATA_MAX-1;
    }
    memcpy(sector,data,size);
    if(!ata_write_sector(DISKFS_BASE_LBA+1+(unsigned int)index,sector))
    {
        last_error = DISKFS_ERROR_DATA_WRITE;
        return 0;
    }

    super.entries[index].used = 1;
    super.entries[index].size = size;
    copy_name(super.entries[index].name,name);
    return flush_super();
}

int diskfs_read(char* name,char* out,unsigned int capacity)
{
    int index = find_file(name);
    unsigned char sector[512];
    unsigned int i;
    unsigned int size;

    last_error = DISKFS_ERROR_NONE;
    if(!mounted)
    {
        last_error = DISKFS_ERROR_NOT_MOUNTED;
        return 0;
    }
    if(index<0 || capacity==0)
    {
        return 0;
    }
    if(!ata_read_sector(DISKFS_BASE_LBA+1+(unsigned int)index,sector))
    {
        last_error = DISKFS_ERROR_DATA_READ;
        return 0;
    }
    size = super.entries[index].size;
    for(i=0;i<size && i<capacity-1;i++)
    {
        out[i] = sector[i];
    }
    out[i] = 0;
    return 1;
}

int diskfs_delete(char* name)
{
    int index = find_file(name);
    unsigned char sector[512];

    last_error = DISKFS_ERROR_NONE;
    if(!mounted)
    {
        last_error = DISKFS_ERROR_NOT_MOUNTED;
        return 0;
    }
    if(index<0)
    {
        return 0;
    }
    clear_sector(sector);
    if(!ata_write_sector(DISKFS_BASE_LBA+1+(unsigned int)index,sector))
    {
        last_error = DISKFS_ERROR_DATA_WRITE;
        return 0;
    }
    memset(&super.entries[index],0,sizeof(DISKFS_ENTRY));
    return flush_super();
}

DISKFS_ENTRY* diskfs_get(unsigned int index)
{
    unsigned int i;
    unsigned int seen = 0;

    for(i=0;i<DISKFS_MAX_FILES;i++)
    {
        if(super.entries[i].used)
        {
            if(seen==index)
            {
                return &super.entries[i];
            }
            seen++;
        }
    }
    return 0;
}

unsigned int diskfs_count()
{
    unsigned int i;
    unsigned int count = 0;

    for(i=0;i<DISKFS_MAX_FILES;i++)
    {
        if(super.entries[i].used)
        {
            count++;
        }
    }
    return count;
}

unsigned int diskfs_mounted()
{
    return mounted;
}

unsigned int diskfs_last_error()
{
    return last_error;
}
