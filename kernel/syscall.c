
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

// TODO
Device* FSDeviceForPath();

int SyscallKPrint(Registers* registers) {
  const char* data = (const char*)registers->ebx;
  KPrint(data);
  return 0;
}

int SyscallExit(Registers* registers) {
  // TODO read exit code from registers->ebx
  Debug("ok\n");
  BYTE processId;
  if (ProcessGetCurrentProcess(&processId) == S_OK) {
    Debug("killing\n");
    ProcessTerminate(processId);
    return 0;
  } else {
    Debug("Could not get current process\n");
    return 1;
  }
}

int SyscallMount(const char* mountPoint, const char* destination) { return 0; }

int SyscallOpen(Registers* registers) {
  const char* pathname = (const char*)registers->ebx;
  int flags = (int)registers->ecx;

  BYTE processId;
  if (ProcessGetCurrentProcess(&processId) == S_OK) {
    Device* device = FSDeviceForPath(pathname);
    if (device == NULL) {
      Debug("Could not find device for path %s\n", pathname);
      return S_FAIL;
    }

    Process* active = ProcessGetActiveProcess();
    int fileSize;
    BYTE* fileData = (BYTE*)device->Read(pathname, &fileSize);
    Debug("syscallopen found '%s'\n", fileData);
    int fd = ProcessOpenFile(processId, pathname, fileData, fileSize);
    Debug("Found fd %d with size %d\n", fd, fileSize);
    return fd;
  } else {
    Debug("Could not get current process\n");
    return -1;
  }
}

int SyscallRead(Registers* registers) {
  int fd = registers->ebx;
  void* buf = (void*)registers->ecx;
  int count = registers->edx;

  Process* p = ProcessGetActiveProcess();

  if (ProcessCanReadFile(p, fd, buf, count) == S_OK) {
    int bytesRead = ProcessReadFile(p->Id, fd, buf, count);
    Debug("Read %d bytes: %s\n", bytesRead, buf);
    registers->eax = bytesRead;
    return bytesRead;
  } else {
    Debug("Can't read, blocking %d at eip %u\n", p->Id, registers->eip);
    p->State = STATE_FOREGROUND_BLOCKED;
    p->IOBlock.Fd = fd;
    p->IOBlock.Buf = buf;
    p->IOBlock.Count = count;
    ProcessSchedule(registers);
    return -1;
  }
} // TODO ssize_t, size_t

int SyscallWrite(Registers* registers) {
  int fd = registers->ebx;
  void* buf = (void*)registers->ecx;
  int count = registers->edx;

  Debug("SyscallWrite! %u, %d bytes\n", buf, count);

  Process* p = ProcessGetActiveProcess();
  if (fd == 1) {
    char* writeBuf = KMalloc(count + 1);
    strcpy(writeBuf, buf, count);
    writeBuf[count] = '\0';
    KPrint(writeBuf);
    Debug("Writing '%s'\n", writeBuf);
  }
  return 0;
} // TODO ssize_t, size_t

int SyscallWaitpid(Registers* registers) {
  int pid = registers->ebx;

  Debug("SyscallWaitpid! %u\n", pid);

  Process* p = ProcessGetActiveProcess();
  p->State = STATE_WAIT_BLOCKED;
  p->WaitpidBlock.id = pid;
  ProcessSchedule(registers);
  return 0;
} // TODO ssize_t, size_t