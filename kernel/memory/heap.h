#ifndef HEAP_H
#define HEAP_H


void heap_init();

void* kmalloc(unsigned int size);

unsigned int heap_get_start();

unsigned int heap_get_end();

unsigned int heap_get_current();

unsigned int heap_get_used();

unsigned int heap_get_free();


#endif
