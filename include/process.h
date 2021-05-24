
#ifndef PROCESS_H
#define PROCESS_H

#include "interrupt.h"
#include "mm.h"
#include "device.h"

#define MAX_PROCESSES 255

#define STATE_PENDING 0
#define STATE_WAITING 1
#define STATE_RUNNING 2
#define STATE_TERMINATING 3
#define STATE_FOREGROUND_BLOCKED 4
#define STATE_WAIT_BLOCKED 5

#define PRIORITY_FOREGROUND 255
#define PRIORITY_BACKGROUND 0

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

#define MAX_FILES_PER_PROCESS 255

typedef struct File_S {
  int FileDescriptor;
  int Flags;
  char Path[255];
  BYTE* Data;
  BYTE* CurrentLocation;
  Device* Device;
  int Size;
} File;

typedef struct IOBlock_S {
  int Fd;
  void* Buf;
  int Count;
} IOBlock;

typedef struct WaitpidBlock_S {
  DWORD id;
  int Status;
} WaitpidBlock;

typedef struct Process_S {
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
  char CommandLine[255];
  PageDirectory* PageDirectory;
  File Files[MAX_FILES_PER_PROCESS];
  char StdinBuffer[1024];
  int StdinPosition;
  IOBlock IOBlock;
  WaitpidBlock WaitpidBlock;
  DWORD ParentId;
  void* CurrentMemPtr;
} Process;

STATUS ProcessSchedule(Registers* registers);
STATUS ProcessInit();
DWORD CreateProcess(void* entryPoint, char* name, BYTE priority,
                    char* commandLine);
STATUS ProcessGetCurrentProcess(BYTE* id);
STATUS ProcessTerminate(BYTE id);
STATUS ProcessGetForegroundProcessId(BYTE* id);
int ProcessOpenFile(BYTE id, char* name, BYTE* fileData, int size);

#endif
