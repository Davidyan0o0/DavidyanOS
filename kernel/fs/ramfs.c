#include "kernel/fs/ramfs.h"

#include "kernel/lib/string.h"
#include "kernel/fs/ramfs_assets.h"

static RAMFS_NODE nodes[RAMFS_FILE_MAX];
static unsigned int mounted = 0;

static void ramfs_clear_node(RAMFS_NODE* node)
{
    memset(node,0,sizeof(RAMFS_NODE));
}

static void ramfs_copy_name(char* dest,char* src)
{
    unsigned int i;

    for(i=0;i<RAMFS_NAME_MAX-1 && src[i];i++)
    {
        dest[i] = src[i];
    }

    dest[i] = 0;
}

static unsigned int ramfs_copy_data(char* dest,char* src)
{
    unsigned int i;

    for(i=0;i<RAMFS_DATA_MAX-1 && src[i];i++)
    {
        dest[i] = src[i];
    }

    dest[i] = 0;
    return i;
}

static unsigned int ramfs_append_data(RAMFS_NODE* node,char* src)
{
    unsigned int i = 0;

    while(node->size<RAMFS_DATA_MAX-1 && src[i])
    {
        node->data[node->size] = src[i];
        node->size++;
        i++;
    }

    node->data[node->size] = 0;
    return i;
}

static unsigned int ramfs_text_size(char* text)
{
    unsigned int size = 0;

    while(text[size])
    {
        size++;
    }

    return size;
}

void ramfs_init()
{
    unsigned int i;

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        ramfs_clear_node(&nodes[i]);
    }

    mounted = 1;
    ramfs_mkdir("/cfg");
    ramfs_mkdir("/log");
    ramfs_create("readme.txt","DavidyanOS");
    ramfs_create("net.cfg","ip=10.0.2.15 gateway=10.0.2.2 dns=10.0.2.3");
    ramfs_create("notes.txt","This filesystem lives in kernel memory.");
    ramfs_create("/log/boot.log","console vga pic idt timer heap fs net ready");
    ramfs_create("logo.ppm","P3 2 2 255 255 0 0 0 255 0 0 0 255 255 255 255 255");
    ramfs_create_external(ramfs_davidyan_ppm_name,ramfs_davidyan_ppm_data,ramfs_davidyan_ppm_size);
}

RAMFS_NODE* ramfs_find(char* name)
{
    unsigned int i;

    if(!mounted)
    {
        return 0;
    }

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(nodes[i].used && strcmp(nodes[i].name,name)==0)
        {
            return &nodes[i];
        }
    }

    return 0;
}

int ramfs_create(char* name,char* data)
{
    unsigned int i;

    if(!mounted || name[0]==0 || ramfs_find(name)!=0)
    {
        return 0;
    }

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(!nodes[i].used)
        {
            nodes[i].used = 1;
            nodes[i].directory = 0;
            nodes[i].external = 0;
            nodes[i].external_data = 0;
            ramfs_copy_name(nodes[i].name,name);
            nodes[i].size = ramfs_copy_data(nodes[i].data,data);
            return 1;
        }
    }

    return 0;
}

int ramfs_create_external(char* name,char* data,unsigned int size)
{
    unsigned int i;

    if(!mounted || name[0]==0 || data==0 || ramfs_find(name)!=0)
    {
        return 0;
    }

    if(size==0)
    {
        size = ramfs_text_size(data);
    }

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(!nodes[i].used)
        {
            nodes[i].used = 1;
            nodes[i].directory = 0;
            nodes[i].external = 1;
            nodes[i].external_data = data;
            nodes[i].size = size;
            nodes[i].data[0] = 0;
            ramfs_copy_name(nodes[i].name,name);
            return 1;
        }
    }

    return 0;
}

int ramfs_mkdir(char* name)
{
    unsigned int i;

    if(!mounted || name[0]==0 || ramfs_find(name)!=0)
    {
        return 0;
    }

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(!nodes[i].used)
        {
            nodes[i].used = 1;
            nodes[i].directory = 1;
            nodes[i].external = 0;
            nodes[i].external_data = 0;
            nodes[i].size = 0;
            ramfs_copy_name(nodes[i].name,name);
            nodes[i].data[0] = 0;
            return 1;
        }
    }

    return 0;
}

int ramfs_touch(char* name)
{
    RAMFS_NODE* node = ramfs_find(name);

    if(node!=0)
    {
        return node->directory ? 0 : 1;
    }

    return ramfs_create(name,"");
}

int ramfs_write(char* name,char* data)
{
    RAMFS_NODE* node = ramfs_find(name);

    if(node==0)
    {
        return ramfs_create(name,data);
    }

    if(node->directory)
    {
        return 0;
    }

    node->external = 0;
    node->external_data = 0;
    node->size = ramfs_copy_data(node->data,data);
    return 1;
}

int ramfs_append(char* name,char* data)
{
    RAMFS_NODE* node = ramfs_find(name);

    if(node==0)
    {
        return ramfs_create(name,data);
    }

    if(node->directory)
    {
        return 0;
    }

    if(node->external)
    {
        node->external = 0;
        node->external_data = 0;
        node->size = 0;
        node->data[0] = 0;
    }

    ramfs_append_data(node,data);
    return 1;
}

int ramfs_read(char* name,char* out,unsigned int capacity)
{
    RAMFS_NODE* node = ramfs_find(name);
    unsigned int i;

    if(node==0 || node->directory || capacity==0)
    {
        return 0;
    }

    for(i=0;i<node->size && i<capacity-1;i++)
    {
        out[i] = node->external ? node->external_data[i] : node->data[i];
    }

    out[i] = 0;
    return 1;
}

int ramfs_stat(char* name,unsigned int* size,unsigned int* directory)
{
    RAMFS_NODE* node = ramfs_find(name);

    if(node==0)
    {
        return 0;
    }

    *size = node->size;
    *directory = node->directory;
    return 1;
}

int ramfs_rename(char* old_name,char* new_name)
{
    RAMFS_NODE* node = ramfs_find(old_name);

    if(node==0 || new_name[0]==0 || ramfs_find(new_name)!=0)
    {
        return 0;
    }

    ramfs_copy_name(node->name,new_name);
    return 1;
}

int ramfs_delete(char* name)
{
    RAMFS_NODE* node = ramfs_find(name);

    if(node==0)
    {
        return 0;
    }

    ramfs_clear_node(node);
    return 1;
}

RAMFS_NODE* ramfs_get(unsigned int index)
{
    unsigned int i;
    unsigned int seen = 0;

    if(!mounted)
    {
        return 0;
    }

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(nodes[i].used)
        {
            if(seen==index)
            {
                return &nodes[i];
            }

            seen++;
        }
    }

    return 0;
}

unsigned int ramfs_count()
{
    unsigned int i;
    unsigned int count = 0;

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(nodes[i].used)
        {
            count++;
        }
    }

    return count;
}

unsigned int ramfs_dir_count()
{
    unsigned int i;
    unsigned int count = 0;

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(nodes[i].used && nodes[i].directory)
        {
            count++;
        }
    }

    return count;
}

unsigned int ramfs_capacity()
{
    return RAMFS_FILE_MAX*RAMFS_DATA_MAX;
}

unsigned int ramfs_used_bytes()
{
    unsigned int i;
    unsigned int bytes = 0;

    for(i=0;i<RAMFS_FILE_MAX;i++)
    {
        if(nodes[i].used)
        {
            bytes += nodes[i].size;
        }
    }

    return bytes;
}

unsigned int ramfs_free_bytes()
{
    return ramfs_capacity()-ramfs_used_bytes();
}
