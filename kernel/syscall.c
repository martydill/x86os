
/*
 * KeMain.c
 * Kernel entry point
 */

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

void SyscallKPrint(const char* data) { KPrint(data); }

void SyscallExit(int code) {
  Debug("ok\n");
  BYTE processId;
  if (ProcessGetCurrentProcess(&processId) == S_OK) {
    Debug("killing\n");
    ProcessTerminate(processId);
  } else {
    Debug("Could not get current process\n");
  }
}

void SyscallMount(const char* mountPoint, const char* destination) {}

int SyscallOpen(const char* pathname, int flags) {
  Debug("SyscallOpen!\n");
  BYTE processId;
  if (ProcessGetCurrentProcess(&processId) == S_OK) {
    int size;
    BYTE* fileData = FloppyReadFile(pathname, &size);
    int fd = ProcessOpenFile(processId, pathname, fileData, size);
    Debug("Found fd %d with size %d\n", fd, size);
    return fd;
  } else {
    Debug("Could not get current process\n");
  }
}

int SyscallRead(Registers* registers) {
  int fd = registers->ebx;
  void* buf = registers->ecx;
  int count = registers->edx;

  Process* p = ProcessGetActiveProcess();

  if (ProcessCanReadFile(p, fd, buf, count) == S_OK) {
    int bytesRead = ProcessReadFile(p->Id, fd, buf, count);
    Debug("Read %d bytes: %s\n", bytesRead, buf);
    registers->eax = bytesRead;
  } else {
    Debug("Can't read, blocking %d at eip %u\n", p->Id, registers->eip);
    p->State = STATE_FOREGROUND_BLOCKED;
    p->IOBlock.Fd = fd;
    p->IOBlock.Buf = buf;
    p->IOBlock.Count = count;
    ProcessSchedule(registers);
  }
} // TODO ssize_t, size_t

int SyscallWrite(Registers* registers) {
  int fd = registers->ebx;
  void* buf = registers->ecx;
  int count = registers->edx;

  Debug("SyscallWrite! %u, %d bytes\n", buf, count);

  Process* p = ProcessGetActiveProcess();
  if (fd == 1) {
    char* writeBuf = KMalloc(count + 1);
    Strcpy(writeBuf, buf, count);
    writeBuf[count] = '\0';
    KPrint(writeBuf);
    Debug("Writing '%s'\n", writeBuf);
  }
} // TODO ssize_t, size_t

int SyscallWaitpid(Registers* registers) {
  int pid = registers->ebx;

  Debug("SyscallWaitpid! %u\n", pid);

  Process* p = ProcessGetActiveProcess();
  p->State = STATE_WAIT_BLOCKED;
  p->WaitpidBlock.id = pid;
  ProcessSchedule(registers);
} // TODO ssize_t, size_t