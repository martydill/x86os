
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
#define STATE_SLEEPING 6

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
#define MAX_PATH 255

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
  ProcessId Id;
  int Status;
} WaitpidBlock;

typedef struct SleepBlock_S {
  DWORD SleepUntilTicks;
} SleepBlock;

typedef struct Environment_S {
  char WorkingDirectory[MAX_PATH];
} Environment;

typedef struct Process_S {
  BYTE Priority;
  BYTE State;
  ProcessId Id;

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
  SleepBlock SleepBlock;

  Environment Environment;
  DWORD ParentId;
  void* CurrentMemPtr;
} Process;

typedef struct ProcessList {
  struct ProcessList* Next;
  struct ProcessList* Prev;
  Process* Process;
} ProcessList;

STATUS ProcessSchedule(Registers* registers);
STATUS ProcessInit();
ProcessId CreateProcess(void* entryPoint, char* name, BYTE priority,
                        char* commandLine);
STATUS ProcessGetCurrentProcess(ProcessId* id);
STATUS ProcessTerminate(ProcessId id);
STATUS ProcessGetForegroundProcessId(ProcessId* id);
int ProcessOpenFile(ProcessId id, char* name, BYTE* fileData, int size);
STATUS ProcessSleep(Process* process, unsigned int seconds);
ProcessList* ProcessGetProcesses();
ProcessList* ProcessGetProcessListNodeById(ProcessId id);

Process* ProcessGetActiveProcess();

const char* ProcessStatusToString(Process* process);

#endif
