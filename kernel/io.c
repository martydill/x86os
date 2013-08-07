
/*
* IoMain.c
* Misc. I/O functions
*/

#include <kernel.h>


/* Reads a byte from the specified port */
BYTE IoReadPortByte(unsigned short port)
{
    BYTE b;

    __asm__ __volatile__ (
        "inb %%dx, %%al"
        :"=a" (b)
        :"d" (port)
    );

    return b;
}


/* Reads a word from the specified port */
WORD IoReadPortWord(unsigned int port)
{
    WORD w;

    __asm__ __volatile__ (
        "inw %%dx, %%ax"
        :"=a" (w)
        :"d" (port)
    );

    return w;
}


/* Reads a doubleword from the specified port */
DWORD IoReadPortDword(unsigned int port)
{
    DWORD d;

    __asm__ __volatile__ (
        "inl %%dx, %%eax"
        :"=a" (d)
        :"d" (port)
    );

    return d;
}


/* Write a byte to the specified port */
void IoWritePortByte(unsigned int port, BYTE byte)
{
    __asm__ __volatile__ (
        "outb %%al, %%dx"
        ::"d" (port), "a" (byte)
    );

    return;
}


/* Write a word to the specified port */
void IoWritePortWord(unsigned int port, WORD word)
{
    __asm__ __volatile__ (
        "outw %%ax, %%dx"
        ::"d" (port), "a" (word)
    );

    return;
}


/* Write a doubleword to the specified port */
void IoWritePortDword(unsigned int port, DWORD dword)
{
    __asm__ __volatile__ (
        "outl %%eax, %%dx"
        ::"d" (port), "a" (dword)
    );

    return;
}
