#ifndef STRING_H
#define STRING_H


int strlen(char* text);

int strcmp(char* left,char* right);

int strncmp(char* left,char* right,int count);

void* memset(void* dest,int value,int count);

void* memcpy(void* dest,void* src,int count);


#endif
