
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
#include <floppy.h>

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
  Debug("Cannot convert virtual address %u to physical address",
        virtualAddress);
  KeHalt();
  return -1; // Not reachable
};

extern PageDirectory* kernelPageDirectory;

void KeSysCallHandler(Registers* registers) {
  DWORD syscall = registers->eax;
  Debug("Syscall handler %u %u %u %u %u %u %u %u %u\n", registers->eax,
        registers->ebx, registers->ecx, registers->edx, registers->esi,
        registers->edi, registers->ebp, registers->userEsp, registers->eip);
  if (syscall == SYSCALL_EXIT) {
    registers->eax = SyscallExit(registers);
  } else if (syscall == SYSCALL_KPRINT) {
    registers->eax = SyscallKPrint(registers);
  } else if (syscall == SYSCALL_OPEN) {
    registers->eax = SyscallOpen(registers);
  } else if (syscall == SYSCALL_READ) {
    registers->eax = SyscallRead(registers);
  } else if (syscall == SYSCALL_WRITE) {
    registers->eax = SyscallWrite(registers);
  } else if (syscall == SYSCALL_POSIX_SPAWN) {
    registers->eax = SyscallPosixSpawn(registers);
  } else if (syscall == SYSCALL_WAITPID) {
    SyscallWaitpid(registers);
  } else if (syscall == SYSCALL_OPENDIR) {
    registers->eax = SyscallOpendir(registers);
  } else if (syscall == SYSCALL_READDIR) {
    Debug("SYSCALL_READDIR %d\n", registers->ebx);
    registers->eax = SyscallReaddir(registers);
  } else if (syscall == SYSCALL_CLOSEDIR) {
    Debug("SYSCALL_CLOSEDIR %d\n", registers->ebx);
    // TODO implement this
    registers->eax = SyscallClosedir(registers);
  } else if (syscall == SYSCALL_CHDIR) {
    registers->eax = SyscallChdir(registers);
  } else if (syscall == SYSCALL_GETCWD) {
    Debug("SYSCALL_GETCWD");
    registers->eax = SyscallGetcwd(registers);
  } else if (syscall == SYSCALL_SLEEP) {
    registers->eax = SyscallSleep(registers);
  } else if (syscall == SYSCALL_KILL) {
    Debug("SYSCALL_KILL %u\n", registers->ebx);
    registers->eax = SyscallKill(registers);
  } else if (syscall == SYSCALL_STAT) {
    registers->eax = SyscallStat(registers);
  } else {
    KePanic(registers);
  }
  Debug("Done syscall handler %u, returning to %u for stack %u\n", syscall,
        registers->eip, registers->userEsp);
}

extern void KeSwitchToUserMode();

/* Start here ... */
void KeMain(MultibootInfo* bootInfo) {
  unsigned int amount;
  char* buf;

  InitializeGDT();
  InitializeIDT();
  MMInitializePaging();
  DeviceInit();

  ConInit();
  ConClearScreen();

  KPrint("x86OS kernel\n");

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

  KPrint("Initializing ProcFS...\n");
  ProcFSInit();
  InstallInterruptHandler(0x80, KeSysCallHandler);
  Test_String();

  KeDisableInterrupts();
  KPrint("Initializing VFS\n");
  FSInit();

  int size;
  BYTE* fileData = (BYTE*)FloppyReadFile("idle", &size);
  ELFParseFile(fileData, "idle", "idle", PRIORITY_BACKGROUND);

  fileData = (BYTE*)FloppyReadFile("shell", &size);
  ELFParseFile(fileData, "shell", "shell", PRIORITY_FOREGROUND);

  Debug("Jumping to user mode\n");
  KeSwitchToUserMode();
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
  Dosprintf(sizeof(buffer), buffer, format, args);
  va_end(args);

  ConGetCursorPosition(&point);
  ConDisplayString(buffer, point.X, point.Y);

  return;
}

void KResetCursor() {
  xPos = 0;
  yPos = 0;
}
