
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


void SyscallKPrint(const char* data){
  KPrint(data);
}

void SyscallExit(int code) {
    Debug("ok\n");
    BYTE processId;
    if(ProcessGetCurrentProcess(&processId) == S_OK) {
      Debug("killing\n");
      ProcessTerminate(processId);
    }
    else {
    Debug("Could not get current process\n");
    }
}

void SyscallMount(const char* mountPoint, const char* destination) {

}

int SyscallOpen(const char *pathname, int flags) {
  Debug("SyscallOpen!\n");
    BYTE processId;
    if (ProcessGetCurrentProcess(&processId) == S_OK) {
      int size;
      BYTE* fileData = FloppyReadFile(pathname, &size);
      int fd = ProcessOpenFile(processId, pathname, fileData, size);
      Debug("Found fd %d\n", fd);
      return fd;
    } else {
      Debug("Could not get current process\n");
    }
}

int SyscallRead(int fd, void *buf, int count) {
    Debug("SyscallRead! %u\n", buf);
    BYTE processId;
    if (ProcessGetCurrentProcess(&processId) == S_OK) {
      int bytesRead = ProcessReadFile(processId, fd, buf, count);
      Debug("Read %d bytes\n", bytesRead);
      return bytesRead;
    } else {
      Debug("Could not get current process\n");
    }
}  // TODO ssize_t, size_t
