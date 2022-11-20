#define DEBUG 1
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

// Must match order in kernel_shared.h
const char* SYSCALL_NAMES[] = {
    TEXT(SYSCALL_EXIT),        TEXT(SYSCALL_KPRINT),   TEXT(SYSCALL_MOUNT),
    TEXT(SYSCALL_OPEN),        TEXT(SYSCALL_READ),     TEXT(SYSCALL_WRITE),
    TEXT(SYSCALL_POSIX_SPAWN), TEXT(SYSCALL_WAITPID),  TEXT(SYSCALL_OPENDIR),
    TEXT(SYSCALL_READDIR),     TEXT(SYSCALL_CLOSEDIR), TEXT(SYSCALL_CHDIR),
    TEXT(SYSCALL_GETCWD),      TEXT(SYSCALL_SLEEP),    TEXT(SYSCALL_KILL),
    TEXT(SYSCALL_STAT),        TEXT(SYSCALL_FSTAT)};

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
  ProcessId processId;
  if (ProcessGetCurrentProcess(&processId) == S_OK) {
    int page = MMGetPageForProcess(processId);
    DWORD phys = virtualAddress + (page - 16) * 4 * 1024 * 1024;
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
  Debug("Begin %s %u %u %u %u %u %u %u %u %u\n", SYSCALL_NAMES[syscall],
        registers->eax, registers->ebx, registers->ecx, registers->edx,
        registers->esi, registers->edi, registers->ebp, registers->userEsp,
        registers->eip);

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
    registers->eax = SyscallReaddir(registers);
  } else if (syscall == SYSCALL_CLOSEDIR) {
    // TODO implement this
    registers->eax = SyscallClosedir(registers);
  } else if (syscall == SYSCALL_CHDIR) {
    registers->eax = SyscallChdir(registers);
  } else if (syscall == SYSCALL_GETCWD) {
    registers->eax = SyscallGetcwd(registers);
  } else if (syscall == SYSCALL_SLEEP) {
    registers->eax = SyscallSleep(registers);
  } else if (syscall == SYSCALL_KILL) {
    registers->eax = SyscallKill(registers);
  } else if (syscall == SYSCALL_STAT) {
    registers->eax = SyscallStat(registers);
  } else {
    KePanic(registers);
  }
  Debug("End %s %u, returning to %u for stack %u\n", SYSCALL_NAMES[syscall],
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

  KPrint("Initializing virtual devices...\n");
  ProcFSInit();
  NetFSInit();
  // NullDeviceInit();

  InstallInterruptHandler(0x80, KeSysCallHandler);
  Test_String();
  Test_Path();

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
