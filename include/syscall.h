
#ifndef SYSCALL_H
#define SYSCALL_H
#include <kernel_shared.h>
#include <kernel.h>
#include <interrupt.h>

int SyscallKPrint(Registers* registers);
int SyscallExit(Registers* registers);
int SyscallMount(const char* mountPoint, const char* destination);
int SyscallOpen(Registers* registers);
int SyscallRead(Registers* registers);    // TODO ssize_t, size_t
int SyscallWaitpid(Registers* registers); // TODO ssize_t, size_t

#endif
