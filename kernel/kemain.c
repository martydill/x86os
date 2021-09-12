
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
  if (syscall == SYSCALL_EXIT) {
    SyscallExit(registers);
  } else if (syscall == SYSCALL_KPRINT) {
    SyscallKPrint(registers->ebx);
    registers->eax = 0;
  } else if (syscall == SYSCALL_OPEN) {
    Debug("SYSCALL_OPEN\n");
    int fd = SyscallOpen(registers->ebx, registers->ecx);
    registers->eax = fd;
  } else if (syscall == SYSCALL_READ) {
    Debug("read\n");
    SyscallRead(registers);
  } else if (syscall == SYSCALL_WRITE) {
    SyscallWrite(registers);
  } else if (syscall == SYSCALL_POSIX_SPAWN) {
    DWORD pidPhysicalAddress =
        MMVirtualAddressToPhysicalAddress(registers->ebx);
    DWORD physicalAddress = MMVirtualAddressToPhysicalAddress(registers->ecx);
    DWORD argvAddress = MMVirtualAddressToPhysicalAddress(registers->esi);
    Debug("SYSCALL_POSIX_SPAWN path: %u %s argv: %u %s\n", physicalAddress,
          physicalAddress, argvAddress, argvAddress);
    int size;
    BYTE* fileData = FloppyReadFile(physicalAddress, &size);
    if (fileData == NULL) {
      registers->eax = -1;
    } else {
      Debug("Read %d bytes\n", size);
      // TODO - figure out how to choose foreground for processes requiring i/o
      DWORD childProcessId = ELFParseFile(fileData, physicalAddress,
                                          argvAddress, PRIORITY_BACKGROUND);
      Debug("Started process %d, writing id to %u\n", childProcessId,
            pidPhysicalAddress);
      *(DWORD*)pidPhysicalAddress = childProcessId;
    }
  } else if (syscall == SYSCALL_WAITPID) {
    SyscallWaitpid(registers);
  } else if (syscall == SYSCALL_OPENDIR) {
    const char* dirName =
        (const char*)MMVirtualAddressToPhysicalAddress(registers->ebx);
    Debug("SYSCALL_OPENDIR %s\n", dirName);

    // TODO get device, read from device depending on what dir we are accessing
    Device* device = FSDeviceForPath(dirName);
    Debug("****************%s\n", device->Name);
    Debug("Done find device\n");
    Process* active = ProcessGetActiveProcess();
    struct _DirImpl* dir = KMallocInProcess(active, sizeof(struct _DirImpl));
    device->OpenDir(dirName, dir);
    // FloppyReadDirectory(dirName, dir);
    registers->eax = dir;
    // Debug("Returning %u %d %d\n", registers->eax, dir->Count, dir->Current);
  } else if (syscall == SYSCALL_READDIR) {
    Debug("SYSCALL_READDIR %d\n", registers->ebx);
  } else if (syscall == SYSCALL_CLOSEDIR) {
    Debug("SYSCALL_CLOSEDIR %d\n", registers->ebx);
    // TODO implement this
  } else if (syscall == SYSCALL_CHDIR) {
    Debug("SYSCALL_CHDIR");
    // TODO fix issue here and in other places where params passed to main are
    // kernel addresses not user addresses. Should require a
    // MMVIrtualAddressToPhysicalAddress here.
    char* dirName = (char*)registers->ebx;
    Process* active = ProcessGetActiveProcess();
    Debug("Changing to %s\n", dirName);
    // TODO validate this
    strcpy(&active->Environment.WorkingDirectory, dirName,
           sizeof(active->Environment.WorkingDirectory));
    Debug("WD is now %s\n", active->Environment.WorkingDirectory);
    registers->eax = 1; // TODO return value on failure
  } else if (syscall == SYSCALL_GETCWD) {
    Debug("SYSCALL_GETCWD");
    DWORD physicalAddress = MMVirtualAddressToPhysicalAddress(registers->ebx);
    Process* active = ProcessGetActiveProcess();
    // TODO use caller's length
    Debug("WD was %s\n", active->Environment.WorkingDirectory);
    strcpy((char*)physicalAddress, &active->Environment.WorkingDirectory,
           sizeof(active->Environment.WorkingDirectory));
    Debug("Value is %s\n", physicalAddress);
    // TODO return value on failure
    registers->eax = registers->ebx;

  } else if (syscall == SYSCALL_SLEEP) {
    Debug("SYSCALL_SLEEP %u\n", registers->ebx);
    Process* active = ProcessGetActiveProcess();
    ProcessSleep(active, registers->ebx);
    registers->eax = 0; // TODO return value for signals
    ProcessSchedule(registers);
  } else if (syscall == SYSCALL_KILL) {
    Debug("SYSCALL_KILL %u\n", registers->ebx);
    if (ProcessTerminate(registers->ebx) == S_OK) {
      Debug("Killed %d\n", registers->ebx);
      registers->eax = 0;
    } else {
      Debug("Kill %d failed\n", registers->ebx);
      registers->eax = -1;
    }
  } else if (syscall == SYSCALL_STAT) {
    DWORD physicalAddress =
        registers->ebx; // MMVirtualAddressToPhysicalAddress(registers->ebx); //
                        // TODO why is this not a virtual address...
    char* name = (char*)physicalAddress;
    DWORD statbufPhysicalAddress =
        MMVirtualAddressToPhysicalAddress(registers->ecx);
    struct stat* statbuf = (struct stat*)statbufPhysicalAddress;
    Debug("SYSCALL_STAT: %d %s %d\n", registers->ebx, name,
          statbufPhysicalAddress);

    Device* device = FSDeviceForPath(name);
    Debug("****************%s\n", device->Name);
    Debug("Done find device\n");

    // Process* active = ProcessGetActiveProcess();
    // TODO cache this

    // struct _DirImpl* dir = KMallocInProcess(active, sizeof(struct _DirImpl));
    STATUS result = device->Stat(name, statbuf);

    // // TODO cache this
    // struct _DirImpl* dir = KMallocInProcess(active, sizeof(struct _DirImpl));
    // // TODO fix directory
    // // TOOO read from device/procfs
    // FloppyReadDirectory("/", dir);
    // for(int i = 0; i < dir->Count; ++i) {
    //     if(!strcmp(physicalAddress, dir->dirents[i].d_name)) {
    // 	    // TODO fill in other members of statbuf
    // 	    statbuf->st_mode = dir->dirents[i].st_mode;
    // 	    statbuf->st_size = dir->dirents[i].st_size;
    // 	    registers->eax = 0;
    // 	    return;
    //     }
    // }
    registers->eax = result;
  } else {
    KePanic(registers);
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

  KPrint("Initializing ProcFS...\n");
  ProcFSInit();
  InstallInterruptHandler(0x80, KeSysCallHandler);
  Test_String();

  KeDisableInterrupts();
  KPrint("Initializing VFS\n");
  FSInit();

  int size;
  BYTE* fileData = FloppyReadFile("idle", &size);
  ELFParseFile(fileData, "idle", "idle", PRIORITY_BACKGROUND);

  fileData = FloppyReadFile("shell", &size);
  ELFParseFile(fileData, "shell", "shell", PRIORITY_FOREGROUND);

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
