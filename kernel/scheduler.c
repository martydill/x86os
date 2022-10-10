#include <kernel.h>
#include <process.h>
#include <mm.h>
#include <kernel_shared.h>

// Stack starts at 64 MB + 8k
#define VIRTUAL_STACK_ADDRESS (64 * 1024 * 1024 + 8 * 1024)

ProcessId nextId = 1;
BYTE processCount = 0;

// Must match order of defines in process.h
const char* const ProcessStateNames[] = {
    "pending",      "waiting", "running", "terminating", "foreground_blocked",
    "wait_blocked", "sleeping"};

void MMMap(PageDirectory* pageDirectory, int physicalPage, int virtualPage,
           ProcessId processId);

Process* foreground = NULL;
ProcessList* processListStart = NULL;
ProcessList* processListEnd = NULL;
Process* active = NULL;

ProcessId CreateProcess(void* entryPoint, char* name, BYTE priority,
                        char* commandLine) {
  Process* p = (Process*)KMalloc(sizeof(Process));
  Memset((BYTE*)p, 0, sizeof(Process));
  p->Id = nextId++;
  p->State = STATE_PENDING;
  p->Entry = (unsigned int)entryPoint;
  p->Priority = priority;
  p->KernelStack = VIRTUAL_STACK_ADDRESS;
  p->PageDirectory =
      (PageDirectory*)KMallocWithTagAligned(sizeof(PageDirectory), "BASE",
                                            4096); // & 0xFFFFF000;
  p->ParentId = active != NULL ? active->Id : 0;

  int page = MMGetNextFreePage();

  // Physical address of memory used for heap allocations
  // TODO move to MM
  p->CurrentMemPtr = page * 4 * 1024 * 1024 + 1024 * 1024;

  Debug("Process id %d, page %u, st: %u, memptr %u\n", p->Id, page,
        p->KernelStack, p->CurrentMemPtr);
  if (active != NULL) {
    // Copy parent's environment to child
    Memcopy((BYTE*)&p->Environment, (BYTE*)&active->Environment,
            sizeof(Environment));
  } else {
    strcpy(p->Environment.WorkingDirectory, "/", 1);
  }

  Debug("Process id %d, memptr %u\n", p->Id, p->CurrentMemPtr);
  MMInitializePageDirectory(p->PageDirectory);
  MMMap(p->PageDirectory, 16, page, p->Id);
  MMAllocatePageToProcess(page, p->Id);

  Debug("Commandline: %s\n", commandLine);
  strcpy(&p->Name, name, strlen(name));
  strcpy(&p->CommandLine, commandLine, strlen(commandLine));

  if (processListStart->Process == NULL) {
    Debug("Updating existing process list node %u %u %u\n", p, processListStart,
          processListStart->Next);
    processListStart->Process = p;
    ++processCount;
  } else {
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
    Debug("Created %u %u %u %u %u %s %u\n", entryPoint, p, next,
          processListStart, processListEnd, name, p->CurrentMemPtr);
  }

  if (priority == PRIORITY_FOREGROUND) {
    foreground = p;
  }
  return p->Id;
}

DWORD LastTicks;

STATUS ProcessInit() {
  processListStart = KMalloc(sizeof(ProcessList));
  processListStart->Next = NULL;
  processListStart->Prev = NULL;
  processListStart->Process = NULL;
  processListEnd = processListStart;

  LastTicks = TimerGetTicks();
  return S_OK;
}

ProcessList* ProcessGetProcessListNodeById(ProcessId id) {
  // Debug("Searching for process %d\n", id);
  ProcessList* ps = processListStart;
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
  do {
    // Debug("%d:  %s Self: %u  Next: %u Prev: %u  eip: %u  esp: %u  edx%u\n",
    // count, node->Process->Name, node, node->Next, node->Prev,
    // node->Process->Registers.eip, node->Process->Registers.userEsp,
    // node->Process->Registers.edx);
    node = node->Next;
    count++;
  } while (node != NULL);
}

STATUS ProcessSchedule(Registers* registers) {

  ProcessList* node;
  if (processListStart->Process == NULL) {
    Debug("No processes yet\n");
    return S_FAIL;
  }

  DumpProcesses();
  if (active) {
    node = ProcessGetProcessListNodeById(active->Id);
    if (node == NULL) {
      return S_FAIL;
    }
  }

  DWORD currentTicks = TimerGetTicks();
  if (active) {
    if (active->State == STATE_TERMINATING) {
      Debug("%s %d died\n", active->Name, active->CpuTicks);

      int pageForProcess = MMGetPageForProcess(active->Id);
      MMFreePage(pageForProcess);

      Debug("ParentId was %d\n", active->ParentId);
      if (active->ParentId != 0) {
        ProcessList* node = ProcessGetProcessListNodeById(active->ParentId);
        Debug("waiting on %d\n", node->Process->WaitpidBlock.Id);
        if (node && node->Process->WaitpidBlock.Id == active->Id) {
          node->Process->WaitpidBlock.Id = 0;
          node->Process->State = STATE_WAITING;
          Debug("%s %d waitpid is unblocked\n", node->Process->Name,
                node->Process->Id);
        }
      }

      processCount--;

      if (node->Next) {
        node->Next->Prev = node->Prev;
      }
      if (node->Prev) {
        node->Prev->Next = node->Next;
        processListEnd = node->Prev;

        if (processListEnd == node) {
          processListEnd = node->Prev;
        }
      }
      Debug("Removed from list\n");
      if (active == foreground) {
        // foreground = NULL;
        Debug("This was the foreground process, will find a new one\n");
        // TODO switch to parent
        // For now just find process 2 - shell
        foreground = processListStart->Next->Process;
      }
      active = NULL;

    } else {
      // todo don't switch to idle if we are still running
      // Debug("Switching from %s %d\n", active->Name, active->State);
      // Debug("Old values: Esp %u Eip %u Eax %u Ebx %u Ecx %u Edx %u\n",
      //       active->Registers.userEsp, active->Registers.eip,
      //       active->Registers.eax, active->Registers.ebx,
      //       active->Registers.ecx, active->Registers.edx);
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
  if (active) {
    while (node) {
      node = node->Next;
      if (node != NULL && node->Process->State != STATE_FOREGROUND_BLOCKED &&
          node->Process->State != STATE_WAIT_BLOCKED &&
          node->Process->State != STATE_SLEEPING) {
        // Debug("Switching to next process %s %u\n", node->Process->Name,
        //      node->Process->Id);
        active = node->Process;
        break;
      } else if (node != NULL &&
                 (node->Process->State == STATE_FOREGROUND_BLOCKED ||
                  node->Process->State == STATE_WAIT_BLOCKED)) {
        // Debug("Next process %s is blocked \n", node->Process->Name);
      } else if (node != NULL && node->Process->State == STATE_SLEEPING &&
                 node->Process->SleepBlock.SleepUntilTicks < currentTicks) {
        Debug("Waking from sleep");
        active = node->Process;
        break;
      }
    }
    if (node == NULL) {
      // Debug("Switching to first process %s\n",
      // processListStart->Process->Name);
      active = processListStart->Process;
    }
  }

  else {
    Debug("Switching to real first process %s\n",
          processListStart->Process->Name);
    active = processListStart->Process;
  }

  if (active) {
    if (active->State == STATE_PENDING) {
      Debug("Command line: %s\n", active->CommandLine);
      BYTE argcCounter = 1;
      char* c = active->CommandLine;
      while (*c) {
        if (*c++ == ' ') {
          argcCounter++;
        }
      }
      char** p = KMallocInProcess(active, argcCounter * sizeof(char*));
      char* tok = strtok(active->CommandLine, ' ');
      int cur = 0;

      while (tok) {
        p[cur] = KMallocInProcess(active, strlen(tok) + 1);
        Debug("%u: Found token %s at %u\n", p, tok, p[cur]);
        strcpy(p[cur], tok, strlen(tok) + 1);
        // convert to virtual address
        // TODO use a function
        int page = MMGetPageForProcess(active->Id);
        p[cur] = p[cur] - (page - 16) * 4 * 1024 * 1024;
        cur++;
        tok = strtok(NULL, ' ');
      }
      Debug("Arg count is %u\n", argcCounter);

      active->State = STATE_RUNNING;
      Memset((BYTE*)&active->Registers, 0, sizeof(Registers));
      active->Registers.eip = active->Entry;
      active->Registers.userEsp = active->KernelStack;
      active->Registers.eax = argcCounter;
      active->Registers.ebx = (unsigned int)p;
    }
    registers->eax = active->Registers.eax;
    registers->ebx = active->Registers.ebx;
    registers->ecx = active->Registers.ecx;
    registers->edx = active->Registers.edx;
    registers->edi = active->Registers.edi;
    registers->esi = active->Registers.esi;
    registers->ebp = active->Registers.ebp;
    registers->userEsp = active->Registers.userEsp;
    registers->eip = active->Registers.eip;

    // Debug("Switching to name: %s id: %d\n", active->Name, active->Id);
    // Debug("Esp %u Eip %u Eax %u Ebx %u Ecx %u Edx %u\n",
    //       active->Registers.userEsp, active->Registers.eip,
    //       active->Registers.eax, active->Registers.ebx,
    //       active->Registers.ecx, active->Registers.edx);
    LastTicks = currentTicks;

    MMSetPageDirectory(active->PageDirectory);
  }

  return S_OK;
}

ProcessList* ProcessGetProcesses() { return processListStart; }

Process* ProcessGetActiveProcess() { return active; }

STATUS ProcessGetCurrentProcess(ProcessId* id) {
  if (id == NULL) {
    return S_FAIL;
  }
  if (active == NULL) {
    return S_FAIL;
  }

  *id = active->Id;
  return S_OK;
}

STATUS ProcessGetForegroundProcessId(ProcessId* id) {
  if (id == NULL) {
    return S_FAIL;
  }
  if (foreground == NULL) {
    return S_FAIL;
  }
  *id = foreground->Id;
  return S_OK;
}

STATUS ProcessSetForegroundProcessId(ProcessId id) {
  if (foreground == NULL) {
    return S_FAIL;
  }
  if (foreground->Id == id) {
    return S_FAIL;
  }

  ProcessList* node = ProcessGetProcessListNodeById(id);
  if (node == NULL) {
    return S_FAIL;
  }

  Process* p = node->Process;
  Debug("Activating process %s\n", p->Name);

  return S_OK;
}

STATUS ProcessTerminate(ProcessId id) {
  ProcessList* node;
  if (id == NULL) {
    return S_FAIL;
  }
  node = ProcessGetProcessListNodeById(id);
  if (node == NULL) {
    return S_FAIL;
  }
  Process* p = node->Process;

  Debug("Terminating process %s (id %d)\n", p->Name, p->Id);
  p->State = STATE_TERMINATING;
  return S_OK;
}

STATUS ProcessBlockForIO(ProcessId id) {
  ProcessList* node = ProcessGetProcessListNodeById(id);
  if (node == NULL) {
    return S_FAIL;
  }
  Process* p = node->Process;
  p->State = STATE_FOREGROUND_BLOCKED;
  Debug("Setting process %d to blocked\n", p->Id);
  return S_OK;
}

STATUS ProcessWakeFromIO(ProcessId id, void (*function)(), void* param) {
  ProcessList* node = ProcessGetProcessListNodeById(id);
  if (node == NULL) {
    return S_FAIL;
  }
  Process* p = node->Process;
  p->State = STATE_WAITING;
  p->Registers.eip = (unsigned int)function;
  Debug("Setting process %d to ready from io\n", p->Id);
  return S_OK;
}

int ProcessOpenFile(ProcessId id, char* name, BYTE* fileData, int size) {
  ProcessList* node = ProcessGetProcessListNodeById(id);
  if (node == NULL) {
    return S_FAIL;
  }
  Process* p = node->Process;
  p->Files[0].Data = fileData;
  p->Files[0].CurrentLocation = fileData;
  // p->Files[0].Path = &name;
  p->Files[0].FileDescriptor = 123; // TODO use unique file descriptors
  p->Files[0].Size = size;
  Debug("Opened fd %d with size %u at location %u\n",
        p->Files[0].FileDescriptor, size, fileData);
  return p->Files[0].FileDescriptor;
}

int ProcessReadFile(ProcessId id, int fd, void* buf, int count) {
  Debug("Read file %d %d %u %d\n", id, fd, buf, count);
  ProcessList* node = ProcessGetProcessListNodeById(id);
  if (node == NULL) {
    return S_FAIL;
  }
  Process* p = node->Process;

  if (fd == 0) {
    Debug("Doing read from stdin\n");
    int len = strlen(p->StdinBuffer);
    if (len > count) {
      len = count;
    }
    Debug("%d bytes\n", len);
    // Reading from stdin
    // buf is virtual address
    // convert to physical
    // todo consider process id
    // Mapping 67108864-71303168 to 71303168-75497472 for process 2
    char* addr = buf + (4 * 1024 * 1024);
    Debug("Copy from %u to %u\n", p->StdinBuffer, addr);
    Memcopy((BYTE*)addr, (BYTE*)p->StdinBuffer, len);
    Memset((BYTE*)p->StdinBuffer, 0, 255);
    p->StdinPosition = 0;
    Debug("Completed kernel mode read\n");
    return len;
  } else {
    int bytesInFile =
        p->Files[0].Size - (p->Files[0].CurrentLocation - p->Files[0].Data);
    int bytesToRead = bytesInFile >= count ? count : bytesInFile;

    if (bytesToRead <= 0) {
      return 0;
    }

    Debug("Copying %d bytes out of %d to %u, currently at %d\n", bytesToRead,
          bytesInFile, buf, p->Files[0].CurrentLocation);
    Memcopy(buf, p->Files[0].CurrentLocation, bytesToRead);
    p->Files[0].CurrentLocation += bytesToRead;
    Debug("Done readfile\n");
    return bytesToRead;
  }
}

STATUS ProcessCanReadFile(Process* process, int fd, void* buf, int count) {
  if (fd == 0) {
    if (process->StdinBuffer[process->StdinPosition - 1] == '\n') {
      process->StdinBuffer[process->StdinPosition] = '\0';
      Debug("Process %d can read now: %s\n", process->Id, process->StdinBuffer);
      return S_OK;
    }
  } else {
    // todo fetch fd, see if we can read
    return S_OK;
  }
  return S_FAIL;
}

int ProcessAddToStdinBuffer(char charToAdd) {
  if (!foreground) {
    Debug("No foreground process\n");
    return S_FAIL;
  }
  Debug("Added %c to buffer\n", charToAdd);

  if (charToAdd == 8) {
    // backspace
    foreground->StdinBuffer[--foreground->StdinPosition] = 0;
  } else {
    // other
    foreground->StdinBuffer[foreground->StdinPosition++] = charToAdd;
  }

  Debug("Foreground state is %d\n", foreground->State);
  // See if this unblocked a read from stdin
  if (foreground->State == STATE_FOREGROUND_BLOCKED) {
    Debug("Foreground process is blocked, checking if we can unblock\n");
    IOBlock* block = &foreground->IOBlock;
    if (ProcessCanReadFile(foreground, block->Fd, block->Buf, block->Count) ==
        S_OK) {
      Debug("Read %u %u %u is now unblocked for process %u\n", block->Fd,
            block->Buf, block->Count, foreground->Id);
      Debug("Returning to eip %u\n", foreground->Registers.eip);
      foreground->State = STATE_WAITING;

      int bytesRead =
          ProcessReadFile(foreground->Id, block->Fd, block->Buf, block->Count);
      Debug("Read %d bytes\n", bytesRead);
      foreground->Registers.eax = bytesRead;
    }
  }

  return S_OK;
}

STATUS ProcessSleep(Process* p, unsigned int seconds) {
  if (p == NULL) {
    return S_FAIL;
  }
  p->State = STATE_SLEEPING;
  p->SleepBlock.SleepUntilTicks = TimerGetTicks() + seconds * 100;
  return S_OK;
}

const char* ProcessStatusToString(Process* process) {
  Assert(process != NULL);

  return ProcessStateNames[process->State];
}