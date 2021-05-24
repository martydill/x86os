
/*
 * KeMain.c
 * Kernel entry point
 */
#include "kernel_shared.h"
#include <kernel.h>
#include <boot.h>
#include <console.h>
#include <mm.h>
#include <interrupt.h>
#include <gdt.h>
#include <timer.h>
#include <keyboard.h>
#include <memory.h>
#include <device.h>
#include <fpu.h>
#include <serialport.h>
#include <pci.h>
#include <process.h>
#include <fs.h>
#include <syscall.h>

/* Enable interrupts */
void KeEnableInterrupts(void) {
  __asm__("sti");
  return;
}

/* Disable interrutps */
void KeDisableInterrupts(void) {
  __asm__("cli");
  return;
}

/* Halts the system */
void KeHalt(void) {
  KeDisableInterrupts();
  while (1) {
  }
  // __asm__ ("hlt");
}

/* Dumps registers to the screen and halts */
void KePanic(Registers* registers) {
  ConMoveCursor(0, 20);
  KPrint("Kernel panic, interrupt #%d\n", registers->interruptNumber);
  KPrint("EAX: %u    EBX: %u   ECX: %u   EDX: %u\n", registers->eax,
         registers->ebx, registers->ecx, registers->edx);
  KPrint("EDI: %u    ESI: %u   ESP: %u   EIP: %u\n", registers->edi,
         registers->esi, registers->userEsp, registers->eip);
  KPrint("Done dumping\n");

  KeHalt();
}

void ShellStart();
void IdleLoop();

DWORD MMVirtualAddressToPhysicalAddress(DWORD virtualAddress) {
  BYTE processId;
  if (ProcessGetCurrentProcess(&processId) == S_OK) {
    DWORD phys = virtualAddress + (processId - 1) * 4 * 1024 * 1024;
    Debug("Virt %u = phys %u for process %d\n", virtualAddress, phys,
          processId);
    return phys;
  }
  KePanic("Cannot convert");
};

extern PageDirectory* kernelPageDirectory;

void KeSysCallHandler(Registers* registers) {
  DWORD syscall = registers->eax;
  Debug("Syscall handler %u %u %u %u %u %u %u %u %u\n", registers->eax,
        registers->ebx, registers->ecx, registers->edx, registers->esi,
        registers->edi, registers->ebp, registers->userEsp, registers->eip);
  if (registers->eax == SYSCALL_EXIT) {
    SyscallExit(registers);
  } else if (registers->eax == SYSCALL_KPRINT) {
    SyscallKPrint(registers->ebx);
    registers->eax = 0;
  } else if (registers->eax == SYSCALL_OPEN) {
    int fd = SyscallOpen(registers->ebx, registers->ecx);
    registers->eax = fd;
  } else if (registers->eax == SYSCALL_READ) {
    Debug("read\n");
    SyscallRead(registers);
  } else if (registers->eax == SYSCALL_WRITE) {
    SyscallWrite(registers);
  } else if (registers->eax == SYSCALL_POSIX_SPAWN) {
    DWORD pidPhysicalAddress =
        MMVirtualAddressToPhysicalAddress(registers->ebx);
    DWORD physicalAddress = MMVirtualAddressToPhysicalAddress(registers->ecx);
    DWORD argvAddress = MMVirtualAddressToPhysicalAddress(registers->esi);
    Debug("SYSCALL_POSIX_SPAWN path: %u %s argv: %u %s\n", physicalAddress,
          physicalAddress, argvAddress, argvAddress);
    int size;
    BYTE* fileData = FloppyReadFile(physicalAddress, &size);
    Debug("Read %d bytes\n", size);
    DWORD childProcessId = ELFParseFile(fileData, physicalAddress, argvAddress);
    Debug("Started process %d, writing id to %u\n", childProcessId,
          pidPhysicalAddress);
    *(DWORD*)pidPhysicalAddress = childProcessId;
  } else if (registers->eax == SYSCALL_WAITPID) {
    SyscallWaitpid(registers);
  }
  else if(registers->eax == SYSCALL_OPENDIR) {
    const char* dirName = (const char*)MMVirtualAddressToPhysicalAddress(registers->ebx);
    Debug("SYSCALL_OPENDIR %s\n", dirName);
    Process* active = ProcessGetActiveProcess();
    struct _DirImpl* dir = KMallocInProcess(active, sizeof(struct _DirImpl));
    FloppyReadDirectory(dirName, dir);
    registers->eax = dir;
  }
  else if(registers->eax == SYSCALL_READDIR) {
    Debug("SYSCALL_READDIR %d\n", registers->ebx);
  }
  Debug("Done syscall handler %u, returning to %u for stack %u\n", syscall,
        registers->eip, registers->userEsp);
}

extern void KeSwitchToUserMode();

/* Start here ... */
int KeMain(MultibootInfo* bootInfo) {
  unsigned int amount;
  char* buf;

  InitializeGDT();
  InitializeIDT();
  MMInitializePaging();
  DeviceInit();

  ConInit();
  ConClearScreen();

  KPrint("FizzOS kernel 0.0.2\n");

  KPrint("Initializing timer...\n");
  TimerInit();

  KPrint("Initializing serial port...\n");
  SerialPortInit();

  KPrint("Initializing DMA...\n");
  DmaInit();

  KPrint("Initializing FPU...\n");
  setup_x87_fpu();

  KPrint("Initializing processes...\n");
  ProcessInit();

  KPrint("Initializing keyboard...\n");
  KbInit();
  KeEnableInterrupts();

  KPrint("Initializing floppy drives...\n");
  FloppyInit();

  KPrint("Initializing PCI ...\n");
  PciInit();

  InstallInterruptHandler(0x80, KeSysCallHandler);
  Test_String();

  KeDisableInterrupts();
  KPrint("Initializing VFS\n");
  FSInit();

  int size;
  BYTE* fileData = FloppyReadFile("idle", &size);
  ELFParseFile(fileData, "idle", "idle");

  fileData = FloppyReadFile("shellx", &size);
  ELFParseFile(fileData, "shellx", "shellx");

  Debug("Jumping to user mode\n");
  KeSwitchToUserMode();
  return 0;
}

void IdleLoop() {
  while (1) {
  }
}

volatile int xPos = 0;
volatile int yPos = 0;

void KPrint(const char* format, ...) {
  va_list args;
  char buffer[8192]; // TODO make dynamic
  POINT point;

  va_start(args, format);
  DoSprintf(sizeof(buffer), buffer, format, args);
  va_end(args);

  ConGetCursorPosition(&point);
  ConDisplayString(buffer, point.X, point.Y);

  return;
}

void KResetCursor() {
  xPos = 0;
  yPos = 0;
}
