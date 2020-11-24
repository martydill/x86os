
/*
* kernel.h
* Basic types and definitions used by the kernel
*/

#ifndef KERNEL_H
#define KERNEL_H


#define NULL  (0)


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef unsigned int STATUS;
typedef unsigned char BOOL;

#define FALSE	0
#define TRUE	1

#define S_OK	0
#define S_FAIL	-1
#define S_BLOCKED 1

typedef struct sPOINT
{
    WORD X;
    WORD Y;
} POINT;

/* Pull in a bunch of standard includes */
#include <assert.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>


void KPrint(const char* format, ...);
void KeEnableInterrupts();

void SerialPortWriteString(const char* format, ...);


#ifdef DEBUG
#define Debug(args...)  SerialPortWriteString(args)
#else
#define Debug(args...)
#endif


#endif
