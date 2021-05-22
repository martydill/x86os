#include <kernel.h>
#include <process.h>
#include <mm.h>

BYTE currentProcess =0;
BYTE processCount = 0;
BYTE nextId = 1;

void MMMap(PageDirectory* pageDirectory, int physicalPage, int virtualPage);

typedef struct ProcessList
{
  struct ProcessList* Next;
  struct ProcessList* Prev;
  Process* Process;
} ProcessList;

// Process processes[MAX_PROCESSES];

Process* foreground = NULL;
ProcessList* processListStart = NULL;
ProcessList* processListEnd = NULL;
STATUS CreateProcess(void* entryPoint, char* name, BYTE priority, char* commandLine)
{
  unsigned int* stackAddress = 64 * 1024 * 1024 + 128 * 1024;
  // Memset(stackAddress - 1024*1024, 0, 1024*1024);
  Process* p = KMalloc(sizeof(Process));
  Memset(p, 0, sizeof(Process));
  p->Id = nextId++;
  p->State = STATE_PENDING;
  p->Entry = entryPoint;
  p->Priority = priority;
  p->KernelStack = stackAddress;
  p->PageDirectory = ((unsigned int) KMallocWithTagAligned(sizeof(PageDirectory), "BASE", 4096));// & 0xFFFFF000;
  Debug("%d Stack address %u contains value %u\n", p->Id, stackAddress, *stackAddress);

  MMInitializePageDirectory(p->PageDirectory);
  MMMap(p->PageDirectory, 16, 7 + p->Id);

  Debug("Commandline: %s\n", commandLine);
  Strcpy(&p->Name, name, Strlen(name));
  Strcpy(&p->CommandLine, commandLine, Strlen(commandLine));

  if(processListStart->Process == NULL) {
    Debug("Updating existing process list node %u %u %u\n", p, processListStart, processListStart->Next);
    processListStart->Process = p;
    ++processCount;
  }
  else {
    Debug("Creating new process list node\n");
    Debug("Old end was %d\n", processListEnd->Process->Id);
    ProcessList* next = KMalloc(sizeof(ProcessList));
    next->Next = NULL;
    next->Prev = processListEnd;
    processListEnd->Next = next;
    next->Process = p;
    processListEnd = next;
    ++processCount;
  
    Debug("New end is %d\n", processListEnd->Process->Id);
    Debug("Created %u %u %u %u %u %s\n", entryPoint, p, next, processListStart, processListEnd, name);
  }

  // if(priority == PRIORITY_FOREGROUND) {
    foreground = p;
  // }

  int counter = 0;
}


Process* active = NULL; //&pr1;

DWORD LastTicks;

STATUS ProcessInit()
{
  processListStart = KMalloc(sizeof(ProcessList));
  processListStart->Next = NULL;
  processListStart->Prev = NULL;
  processListStart->Process = NULL;
  processListEnd = processListStart;

  LastTicks = TimerGetTicks();
  return S_OK;
}


ProcessList* ProcessGetProcessListNodeById(BYTE id) {
  // Debug("Searching for process %d\n", id);
  ProcessList *ps = processListStart;
  do {
    // Debug("Checking %d\n", ps->Process ? ps->Process->Id : 0);
    if (ps->Process && ps->Process->Id == id) {
      // Debug("Found match %s\n", ps->Process->Name);
      return ps;
    }

    ps = ps->Next;
  } while (ps != NULL);
  Debug("Could not find it\n");
  return NULL;
}
void DumpProcesses() {
  int count = 0;
  ProcessList* node = processListStart;
  do 
  {
    // Debug("%d:  %s Self: %u  Next: %u Prev: %u  eip: %u  esp: %u  edx%u\n", count, node->Process->Name, node, node->Next, node->Prev, node->Process->Registers.eip, node->Process->Registers.esp, node->Process->Registers.edx);
    node = node->Next;
    count++;
  }while(node != NULL);
}

STATUS ProcessSchedule(Registers* registers) {

  ProcessList* node;
  // Debug("Schedule\n");
  if(processListStart->Process == NULL) {
    Debug("No processes yet\n");
    return S_FAIL;
  }

  DumpProcesses();
  if(active) {
    node = ProcessGetProcessListNodeById(active->Id);
    if(node == NULL) {
      return S_FAIL;
    }
  }

  DWORD currentTicks = TimerGetTicks();
  if(active) {
    if(active->State == STATE_TERMINATING) {
      Debug("%s %d died\n", active->Name, active->CpuTicks);
      processCount--;
      
      if(node->Next) {
        node->Next->Prev = node->Prev;
      }
      if(node->Prev) {
        node->Prev->Next = node->Next;
        processListEnd = node->Prev;

        if(processListEnd == node) {
          processListEnd = node->Prev;
        }
      }
      Debug("Removed from list\n");
      active = NULL;
    }
    else {
      Debug("Switching from %s\n", active->Name);
      Debug("Old values: Esp %u Eip %u Eax %u Ebx %u Ecx %u Edx %u\n", active->Registers.userEsp, active->Registers.eip, active->Registers.eax, active->Registers.ebx, active->Registers.ecx, active->Registers.edx);
      active->Registers.eax = registers->eax;
      active->Registers.ebx = registers->ebx;
      active->Registers.ecx = registers->ecx;
      active->Registers.edx = registers->edx;

      active->Registers.edi = registers->edi;
      active->Registers.esi = registers->esi;
      active->Registers.ebp = registers->ebp;
      active->Registers.userEsp = registers->userEsp;
      active->Registers.eip = registers->eip;
      active->CpuTicks += (currentTicks - LastTicks);
      // active->State = STATE_WAITING;
    }
  }
  // Memcopy(registers, &active->Registers, sizeof(Registers));

  // if(active == &pr1) {
  //   active = &pr2;
  // }
  // else {
  //   active = &pr1;
  // }
  if(active) {
    while(node) {
      node = node->Next;
      if(node != NULL && node->Process->State != STATE_FOREGROUND_BLOCKED) {
        Debug("Switching to next process %s %u\n", node->Process->Name, node->Process->Id);
        active = node->Process;
        break;
      }
    }
    if(node == NULL) {
      Debug("Switching to first process %s\n", processListStart->Process->Name);
      active = processListStart->Process;
    }
  }
  
  else {
      Debug("Switching to real first process %s\n", processListStart->Process->Name);
    active = processListStart->Process;
  }

  // if (processCount > 0) {
  //   if (foreground == active) {
  //     currentProcess = currentProcess + 1;
  //     if (currentProcess >= processCount) {
  //       currentProcess = 0;
  //     }
  //     active = &processes[currentProcess];
  //   } else {
  //     active = foreground;
  //   }

  //   // Debug("Switch to process %d\n", currentProcess);
  // }

  if(active) {
    // Debug("Switching to process %s %u %u\n", active->Name, active->Entry, active->State);
    if (active->State == STATE_PENDING) {
      // Debug("Making process running");
      Debug("Command line: %s\n", active->CommandLine);
      BYTE argcCounter = 1;
      char* c = active->CommandLine;
      while(*c) {
        if(*c++ == ' ') {
          argcCounter++;
        }
      }
    char**p = KMalloc(argcCounter * sizeof(char*));
    char* tok = strtok(active->CommandLine, ' ');
    int cur = 0;
    while(tok) {
        Debug("Found token %s\n", tok);
        p[cur] = KMalloc(Strlen(tok) + 1);
        Strcpy(p[cur], tok, Strlen(tok) + 1);
        cur++;
        tok = strtok(NULL, ' ');
      }

      active->State = STATE_RUNNING;
      Memset(&active->Registers, 0, sizeof(Registers));
      active->Registers.eip = active->Entry;
      active->Registers.userEsp = active->KernelStack;
      active->Registers.eax = argcCounter;
      active->Registers.ebx = p;
      Debug("Stack is %u\n, count is %d\n", active->Registers.userEsp, argcCounter);
    }
    registers->eax = active->Registers.eax;
    registers->ebx= active->Registers.ebx;
    registers->ecx = active->Registers.ecx;
    registers->edx = active->Registers.edx;
    registers->edi = active->Registers.edi;
    registers->esi = active->Registers.esi;
    registers->ebp = active->Registers.ebp;
    registers->userEsp = active->Registers.userEsp;
    registers->eip = active->Registers.eip;

    Debug("Switching to name: %s id: %d\n", active->Name, active->Id);
    Debug("Esp %u Eip %u Eax %u Ebx %u Ecx %u Edx %u\n", active->Registers.userEsp, active->Registers.eip, active->Registers.eax, active->Registers.ebx, active->Registers.ecx, active->Registers.edx);
    LastTicks = currentTicks;

    MMSetPageDirectory(active->PageDirectory);
  }
}

ProcessList* ProcessGetProcesses(){
  return processListStart;
}

Process* ProcessGetActiveProcess() {
  return active;
}

STATUS ProcessGetCurrentProcess(BYTE* id) {
  if(id == NULL) {
    return S_FAIL;
  }
  if(active == NULL) {
    return S_FAIL;
  }

  *id = active->Id;
  return S_OK;
}

STATUS ProcessGetForegroundProcessId(BYTE* id) {
  if(id == NULL) {
    return S_FAIL;
  }
  if(foreground == NULL) {
    return S_FAIL;
  } 
  *id = foreground->Id;
  return S_OK;
}



STATUS ProcessSetForegroundProcessId(BYTE id) {
  if(foreground == NULL) {
    return S_FAIL;
  } 
  if(foreground->Id == id) {
    return S_FAIL;
  }

  ProcessList* node = ProcessGetProcessListNodeById(id);
  if(node == NULL) {
    return S_FAIL;
  }

  Process* p = node->Process;
  Debug("Activating process %s\n", p->Name);
 
  return S_OK;
}

STATUS ProcessTerminate(BYTE id) {
  ProcessList* node;
  if (id == NULL) {
    return S_FAIL;
  }
  node = ProcessGetProcessListNodeById(id);
  if(node == NULL) {
    return S_FAIL;
  }
  Process *p = node->Process;

  Debug("Terminating process %s\n", p->Name);
  p->State = STATE_TERMINATING;
  return S_OK;
}

STATUS ProcessBlockForIO(BYTE id) {
  ProcessList* node = ProcessGetProcessListNodeById(id);
  if(node == NULL) {
    return S_FAIL;
  }
  Process *p = node->Process;
  p->State = STATE_FOREGROUND_BLOCKED;
  Debug("Setting process %d to blocked\n", p->Id);
  return S_OK;
}


STATUS ProcessWakeFromIO(BYTE id, void (*function)(), void* param) {
  ProcessList* node = ProcessGetProcessListNodeById(id);
  if(node == NULL) {
    return S_FAIL;
  }
  Process *p = node->Process;
  p->State = STATE_WAITING;
  p->Registers.eip = function;
  Debug("Setting process %d to ready from io\n", p->Id);
  return S_OK;
}


int ProcessOpenFile(BYTE id, char* name, BYTE* fileData, int size)
{
   ProcessList* node = ProcessGetProcessListNodeById(id);
  if(node == NULL) {
    return S_FAIL;
  }
  Process *p = node->Process;
  p->Files[0].Data = fileData;
  p->Files[0].CurrentLocation = fileData;
  // p->Files[0].Path = &name;
  p->Files[0].FileDescriptor = 123;
  p->Files[0].Size = size;
  Debug("Opened fd %d\n", p->Files[0].FileDescriptor);
  return p->Files[0].FileDescriptor;
}

int ProcessReadFile(BYTE id, int fd, void* buf, int count)
{
  Debug("Read file %d %d %u %d\n", id, fd, buf, count);
   ProcessList* node = ProcessGetProcessListNodeById(id);
  if(node == NULL) {
    return S_FAIL;
  }
  Process *p = node->Process;

  if(fd == 0) {
    Debug("Doing read from stdin\n");
    int len = Strlen(p->StdinBuffer);
    if(len > count) {
      len = count;
    }
    Debug("%d bytes\n", len);
    // Reading from stdin
    char* addr = buf - (28 * 1024 * 1024);
    Memcopy(addr, p->StdinBuffer, len);
    Memset(p->StdinBuffer, 0, 255);
    p->StdinPosition = 0;
    Debug("Completed kernel mode read\n");
    return len;
  }
  else {
    int bytesInFile = p->Files[0].Size - (p->Files[0].CurrentLocation - p->Files[0].Data);
    int bytesToRead = bytesInFile >= count ? count : bytesInFile;

    if(bytesToRead <= 0) {
      return 0;
    }

    Debug("Copying %d bytes out of %d to %u, currently at %d\n", bytesToRead, bytesInFile, buf, p->Files[0].CurrentLocation);
    Memcopy(buf, p->Files[0].CurrentLocation, bytesToRead);
    p->Files[0].CurrentLocation += bytesToRead;
    Debug("Done readfile\n");
    return bytesToRead;
  }
}


STATUS ProcessCanReadFile(Process* process, int fd, void* buf, int count) {
  if(fd == 0) {
    if(process->StdinBuffer[process->StdinPosition - 1] == '\n') {
      process->StdinBuffer[process->StdinPosition] = '\0';
      Debug("Process %d can read now: %s\n", process->Id, process->StdinBuffer);
      return S_OK;
    }
  }
  
  return S_FAIL;
}

int ProcessAddToStdinBuffer(char charToAdd) {
  if(!foreground) {
    Debug("No foreground process\n");
    return S_FAIL;
  }
  Debug("Added %c to buffer\n", charToAdd);

  foreground->StdinBuffer[foreground->StdinPosition++] = charToAdd;

  // See if this unblocked a read from stdin
  if(foreground->State == STATE_FOREGROUND_BLOCKED) {
    Debug("Foreground process is blocked, checking if we can unblock\n");
    IOBlock* block = &foreground->IOBlock;
     if(ProcessCanReadFile(foreground, block->Fd, block->Buf, block->Count) == S_OK) {
       Debug("Read %u %u %u is now unblocked for process %u\n", block->Fd, block->Buf, block->Count, foreground->Id);
       foreground->State = STATE_WAITING;

      int bytesRead = ProcessReadFile(foreground->Id, block->Fd, block->Buf, block->Count);
      Debug("Read %d bytes\n", bytesRead);
      foreground->Registers.eax = bytesRead;
     }
  }

  return S_OK;
}