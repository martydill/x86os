#include <kernel.h>
#include <process.h>

const MAX_PROCESSES = 256;
BYTE currentProcess =0;
const STATE_PENDING = 0;
const STATE_WAITING = 1;
const STATE_RUNNING = 2;

Process processes[MAX_PROCESSES];


STATUS CreateProcess(void* entryPoint)
{

}

void p1() {
  int counter = 100;
  while(1) {
    Debug("%d ", counter);
    for(int i = 0; i < 10000000; ++i) {

    }
    counter++;
  }
}

void p2() {
  int counter = 888;
  while (1) {
    Debug("%d ", counter);
    for(int i = 0; i < 10000000; ++i) {

    }
    counter++;
  }
}

Process pr1;
Process pr2;
Process* active = &pr1;


STATUS ProcessInit()
{
  Memset(&pr1, 0, sizeof(Process)); 
  Memset(&pr2, 0, sizeof(Process)); 
  pr1.Entry = p1;
  pr2.Entry = p2;
  pr1.State = STATE_PENDING;
  pr2.State = STATE_PENDING;

  return S_OK;
}

STATUS ProcessSchedule(Registers* registers)
{
  active->Registers.eax = registers->eax;
  active->Registers.ebx = registers->ebx;
  active->Registers.ecx = registers->ecx;
  active->Registers.edx = registers->edx;

  active->Registers.edi = registers->edi;
  active->Registers.esi = registers->esi;
  active->Registers.ebp = registers->ebp;
  active->Registers.esp = registers->esp;
  active->Registers.eip = registers->eip;

  // Memcopy(registers, &active->Registers, sizeof(Registers));

  if(active == &pr1) {
    active = &pr2;
  }
  else {
    active = &pr1;
  }

  if (active->State == STATE_PENDING) {
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


  Debug("Switching to %d\n", active);
  Debug("Eip %d Eax %d Ebx %d Ecx %d Edx %d\n", active->Registers.eip, active->Registers.eax, active->Registers.ebx, active->Registers.ecx, active->Registers.edx);
}
