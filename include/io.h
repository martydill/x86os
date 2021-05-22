
/*
 * io.h
 * Definitions of I/O functions
 */

#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <kernel.h>

BYTE IoReadPortByte(unsigned int port);
WORD IoReadPortWord(unsigned int port);
DWORD IoReadPortDword(unsigned int port);

void IoWritePortByte(unsigned int port, BYTE byte);
void IoWritePortWord(unsigned int port, WORD word);
void IoWritePortDword(unsigned int port, DWORD dword);

typedef struct {
} DeviceDriver;

#endif
