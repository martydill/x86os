
#ifndef PROCESS_H
#define PROCESS_H

typedef struct Registers_S
{
    DWORD eax;
    DWORD ebx;
    DWORD ecx;
    DWORD edx;

    DWORD edi;
    DWORD esi;
    DWORD ebp;

} Registers;

typedef struct Process_S
{
    BYTE Priority;
    BYTE State;
    DWORD Id;

    DWORD Esp;
    DWORD SS;
    DWORD KernelStack;
    DWORD UserStack;

    Registers Registers;
} Process;


#endif
