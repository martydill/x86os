
/*
* Gdt.h
* Functions for handling the GDT
*/

#ifndef GDT_H
#define GDT_H


#define LOW_ADDRESS 0
#define HIGH_ADDRESS 0xFFFFFFFF

#define NUM_GDT_ENTRIES		3

struct GDTEntry_S
{
    WORD LowerLimit;
    WORD BaseAddressLow;
    BYTE BaseAddressMiddle;
    BYTE Access;
    BYTE Granularity;
    BYTE BaseAddressHigh;
} __attribute__ ((packed));

typedef struct GDTEntry_S GDTEntry;


struct GDTPointer_S
{
    WORD Limit;
    DWORD Base;
} __attribute__ ((packed));

typedef struct GDTPointer_S GDTPointer;

void InitializeGDT(void);

#endif
