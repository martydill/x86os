
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
    return bytesRead;
  } else {
    Debug("Can't read, blocking %d at eip %u\n", p->Id, registers->eip);
    p->State = STATE_FOREGROUND_BLOCKED;
    p->IOBlock.Fd = fd;
    p->IOBlock.Buf = buf;
    p->IOBlock.Count = count;
    ProcessSchedule(registers);

    // ProcessSchedule can change eax so we need to make sure we return the new
    // value
    // TODO
    return registers->eax;
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

int SyscallReaddir(Registers* registers) { return registers->eax; }

int SyscallClosedir(Registers* registers) { return registers->eax; }

int SyscallWaitpid(Registers* registers) {
  int pid = registers->ebx;

  Debug("SyscallWaitpid! %u\n", pid);

  Process* p = ProcessGetActiveProcess();
  p->State = STATE_WAIT_BLOCKED;
  p->WaitpidBlock.id = pid;
  ProcessSchedule(registers);

  // Return old value of eax here. We do not want to change it because
  // we already saved the value above.
  //   return registers->eax;
  return 0; //
} // TODO ssize_t, size_t

int SyscallKill(Registers* registers) {
  Debug("SYSCALL_KILL %u\n", registers->ebx);
  if (ProcessTerminate(registers->ebx) == S_OK) {
    Debug("Killed %d\n", registers->ebx);
    return 0;
  } else {
    Debug("Kill %d failed\n", registers->ebx);
    return -1;
  }
}

int SyscallSleep(Registers* registers) {
  Debug("SYSCALL_SLEEP %u\n", registers->ebx);
  Process* active = ProcessGetActiveProcess();
  ProcessSleep(active, registers->ebx);
  ProcessSchedule(registers);
  return 0; // TODO return value for signals
}

int SyscallStat(Registers* registers) {
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
  return result;
}

int SyscallChdir(Registers* registers) {
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
  return 1; // TODO return value on failure
}

int SyscallGetcwd(Registers* registers) {
  Debug("SYSCALL_GETCWD");
  DWORD physicalAddress = MMVirtualAddressToPhysicalAddress(registers->ebx);
  Process* active = ProcessGetActiveProcess();
  // TODO use caller's length
  Debug("WD was %s\n", active->Environment.WorkingDirectory);
  strcpy((char*)physicalAddress, &active->Environment.WorkingDirectory,
         sizeof(active->Environment.WorkingDirectory));
  Debug("Value is %s\n", physicalAddress);
  // TODO return value on failure
  return registers->ebx;
}