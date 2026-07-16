#include "kernel/memory/heap.h"


#define HEAP_SIZE (64*1024)


extern unsigned int _kernel_end;


static unsigned int heap_start = 0;
static unsigned int heap_end = 0;
static unsigned int heap_current = 0;


static unsigned int align_up(unsigned int value,unsigned int alignment)
{

    return (value+alignment-1)&~(alignment-1);

}



void heap_init()
{

    heap_start = align_up((unsigned int)&_kernel_end,16);
    heap_current = heap_start;
    heap_end = heap_start+HEAP_SIZE;

}



void* kmalloc(unsigned int size)
{

    unsigned int allocation;


    if(size==0)
    {

        return 0;

    }


    allocation = align_up(heap_current,16);


    if(allocation+size>heap_end)
    {

        return 0;

    }


    heap_current = allocation+size;

    return (void*)allocation;

}



unsigned int heap_get_start()
{

    return heap_start;

}



unsigned int heap_get_end()
{

    return heap_end;

}



unsigned int heap_get_current()
{

    return heap_current;

}



unsigned int heap_get_used()
{

    return heap_current-heap_start;

}



unsigned int heap_get_free()
{

    return heap_end-heap_current;

}
