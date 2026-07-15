#include "kernel/lib/string.h"


int strlen(char* text)
{

    int length = 0;


    while(text[length])
    {

        length++;

    }


    return length;

}



int strcmp(char* left,char* right)
{

    while(*left && *right && *left==*right)
    {

        left++;
        right++;

    }


    return (unsigned char)*left-(unsigned char)*right;

}



int strncmp(char* left,char* right,int count)
{

    int i;


    for(i=0;i<count;i++)
    {

        if(left[i]!=right[i] || left[i]==0 || right[i]==0)
        {

            return (unsigned char)left[i]-(unsigned char)right[i];

        }

    }


    return 0;

}



void* memset(void* dest,int value,int count)
{

    unsigned char* bytes = (unsigned char*)dest;
    int i;


    for(i=0;i<count;i++)
    {

        bytes[i] = (unsigned char)value;

    }


    return dest;

}



void* memcpy(void* dest,void* src,int count)
{

    unsigned char* dest_bytes = (unsigned char*)dest;
    unsigned char* src_bytes = (unsigned char*)src;
    int i;


    for(i=0;i<count;i++)
    {

        dest_bytes[i] = src_bytes[i];

    }


    return dest;

}
