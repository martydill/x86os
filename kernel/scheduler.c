#include <kernel.h>
#include <process.h>
#include <mm.h>

const MAX_PROCESSES = 255;
BYTE currentProcess =0;
BYTE processCount = 0;
const STATE_PENDING = 0;
const STATE_WAITING = 1;
const STATE_RUNNING = 2;
const STATE_TERMINATING = 3;
const PRIORITY_FOREGROUND = 255;
const PRIORITY_BACKGROUND = 0;

// typedef struct
// {
//   ProcessList* Next;
//   ProcessList* Prev;
//   Process* Process;
// } ProcessList;

Process processes[MAX_PROCESSES];

Process* foreground = NULL;

STATUS CreateProcess(void* entryPoint, char* name, BYTE priority)
{
  Process* p = &processes[processCount];
  Memset(p, 0, sizeof(Process));
  p->Id = processCount;
  p->State = STATE_PENDING;
  p->Entry = entryPoint;
  p->Priority = priority;
  MMInitializePageDirectory(&p->PageDirectory);
  // MMInitializePageTables(&p->PageDirectory);
  Strcpy(&p->Name, name, Strlen(name));

  ++processCount;
  Debug("Created %d\n", entryPoint);
  if(priority == PRIORITY_FOREGROUND) {
    foreground = p;
  }
}


Process* active = NULL; //&pr1;

DWORD LastTicks;

STATUS ProcessInit()
{
  // CreateProcess(p1);
  // CreateProcess(p2);

  // Memset(&pr1, 0, sizeof(Process)); 
  // Memset(&pr2, 0, sizeof(Process)); 
  // pr1.Entry = p1;
  // pr2.Entry = p2;
  // pr1.State = STATE_PENDING;
  // pr2.State = STATE_PENDING;
  LastTicks = TimerGetTicks();
  return S_OK;
}

STATUS ProcessSchedule(Registers* registers)
{
  DWORD currentTicks = TimerGetTicks();
  if(active) {
    if(active->State == STATE_TERMINATING) {
      Debug("%s %d died\n", active->Name, active->CpuTicks);
      processCount--;
    }
    else {
      active->Registers.eax = registers->eax;
    active->Registers.ebx = registers->ebx;
    active->Registers.ecx = registers->ecx;
    active->Registers.edx = registers->edx;

    active->Registers.edi = registers->edi;
    active->Registers.esi = registers->esi;
    active->Registers.ebp = registers->ebp;
    active->Registers.esp = registers->esp;
    active->Registers.eip = registers->eip;
    active->CpuTicks += (currentTicks - LastTicks);
    active->State = STATE_WAITING;
    }
  }
  // Memcopy(registers, &active->Registers, sizeof(Registers));

  // if(active == &pr1) {
  //   active = &pr2;
  // }
  // else {
  //   active = &pr1;
  // }
  if(processCount > 0) {
    if (foreground == active) {
      currentProcess = currentProcess + 1;
      if(currentProcess >= processCount) {
        currentProcess = 0;
      }
    active = &processes[currentProcess];
    }
    else {
      active = foreground;
    }

    // Debug("Switch to process %d\n", currentProcess);
  }

  if(active) {
    if (active->State == STATE_PENDING) {
      // Debug("Making process running");
      active->State = STATE_RUNNING;
      active->Registers.eip = active->Entry;
    }

    // Memcopy(&active->Registers, registers, sizeof(Registers));
    registers->eax = active->Registers.eax;
    registers->ebx= active->Registers.ebx;
    registers->ecx= active->Registers.ecx;
    registers->edx = active->Registers.edx;
    registers->edi = active->Registers.edi;
    registers->esi = active->Registers.esi;
    registers->ebp = active->Registers.ebp;
    registers->esp = active->Registers.esp;
    registers->eip = active->Registers.eip;


    // Debug("Switching to %s %d\n", active->Name, active->CpuTicks);
    // Debug("Eip %d Eax %d Ebx %d Ecx %d Edx %d\n", active->Registers.eip, active->Registers.eax, active->Registers.ebx, active->Registers.ecx, active->Registers.edx);
    LastTicks = currentTicks;
  }
}

Process* ProcessGetProcesses(){
  return &processes;
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
  for(BYTE i = 0; i < MAX_PROCESSES; ++i) {
      Process* p = &processes[i];
      if(p->Id == id) {
        Debug("Activating process %s\n", p->Name);
        foreground = p;
        return S_OK;
      }
  }
  return S_FAIL;
}

STATUS ProcessTerminate(BYTE id) {
  if(id == NULL) {
    return S_FAIL;
  }
    
  for(BYTE i = 0; i < MAX_PROCESSES; ++i) {
      Process* p = &processes[i];
      if(p->Id == id) {
        Debug("Terminating process %s\n", p->Name);
        p->State = STATE_TERMINATING;
        return S_OK;
      }
  }
  return S_FAIL;
}
