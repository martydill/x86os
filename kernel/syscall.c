
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

int SyscallPosixSpawn(Registers* registers) {
  DWORD pidPhysicalAddress = MMVirtualAddressToPhysicalAddress(registers->ebx);
  DWORD physicalAddress = MMVirtualAddressToPhysicalAddress(registers->ecx);
  DWORD argvAddress = MMVirtualAddressToPhysicalAddress(registers->esi);
  Debug("SYSCALL_POSIX_SPAWN path: %u %s argv: %u %s\n", physicalAddress,
        physicalAddress, argvAddress, argvAddress);
  int size;
  BYTE* fileData = (BYTE*)FloppyReadFile((char*)physicalAddress, &size);
  if (fileData == NULL) {
    return -1;
  } else {
    Debug("Read %d bytes\n", size);
    // TODO - figure out how to choose foreground for processes requiring i/o
    DWORD childProcessId = ELFParseFile(fileData, physicalAddress, argvAddress,
                                        PRIORITY_BACKGROUND);
    Debug("Started process %d, writing id to %u\n", childProcessId,
          pidPhysicalAddress);
    *(DWORD*)pidPhysicalAddress = childProcessId;
    return 0;
  }
}

int SyscallOpendir(Registers* registers) {
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
  return (DWORD)dir;
  // Debug("Returning %u %d %d\n", registers->eax, dir->Count, dir->Current);
}

int SyscallReaddir(Registers* registers) { return 0; }

int SyscallClosedir(Registers* registers) { return 0; }

int SyscallWaitpid(Registers* registers) {
  int pid = registers->ebx;

  Debug("SyscallWaitpid! %u\n", pid);

  Process* p = ProcessGetActiveProcess();
  p->State = STATE_WAIT_BLOCKED;
  p->WaitpidBlock.id = pid;
  ProcessSchedule(registers);
  return 0;
} // TODO ssize_t, size_t

int SyscallKill(Registers* registers) { return 0; }

int SyscallSleep(Registers* registers) { return 0; }

int SyscallStat(Registers* registers) { return 0; }