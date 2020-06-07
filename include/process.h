
#ifndef PROCESS_H
#define PROCESS_H

#include "interrupt.h"

// typedef struct Registers
// {
//     DWORD edi;
//     DWORD esi;
//     DWORD ebp;
//     DWORD esp;

//     DWORD edx;
//     DWORD ebx;
//     DWORD ecx;
//     DWORD eax;
// }Registers;


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

    DWORD Entry;
    DWORD CpuTicks;
    char Name[32];
} Process;

STATUS ProcessSchedule(Registers* registers);
STATUS ProcessInit();
STATUS CreateProcess(void* entryPoint, char* name, BYTE priority);
STATUS ProcessGetCurrentProcess(BYTE* id);
STATUS ProcessGetForegroundProcessId(BYTE* id);

#endif
