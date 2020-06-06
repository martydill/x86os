#include <kernel.h>
#include <process.h>

const MAX_PROCESSES = 256;
BYTE currentProcess =0;
BYTE processCount = 0;
const STATE_PENDING = 0;
const STATE_WAITING = 1;
const STATE_RUNNING = 2;

// typedef struct
// {
//   ProcessList* Next;
//   ProcessList* Prev;
//   Process* Process;
// } ProcessList;

Process processes[MAX_PROCESSES];

STATUS CreateProcess(void* entryPoint, char* name)
{
  Process* p = &processes[processCount];
  Memset(p, 0, sizeof(Process));
  p->Id = processCount;
  p->State = STATE_PENDING;
  p->Entry = entryPoint;
  Strcpy(&p->Name, name, Strlen(name));

  ++processCount;
  Debug("Created %d\n", entryPoint);
}


void p1() {
  int counter = 100;
  while(1) {
    // Debug("%d ", counter);
    for(int i = 0; i < 10000000; ++i) {

    }
    counter++;
  }
}

void p2() {
  int counter = 888;
  while (1) {
    // Debug("%d ", counter);
    for(int i = 0; i < 10000000; ++i) {

    }
    counter++;
  }
}

// Process pr1;
// Process pr2;
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
  }
  // Memcopy(registers, &active->Registers, sizeof(Registers));

  // if(active == &pr1) {
  //   active = &pr2;
  // }
  // else {
  //   active = &pr1;
  // }
  if(processCount > 0) {
    currentProcess = currentProcess + 1;
    if(currentProcess >= processCount) {
      currentProcess = 0;
    }
    // Debug("Switch to process %d\n", currentProcess);
    active = &processes[currentProcess];
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