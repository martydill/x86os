
#ifndef SYSCALL_H 
#define SYSCALL_H
#include <kernel_shared.h>
#include <kernel.h>
#include <interrupt.h>

void SyscallKPrint(const char* data);
void SyscallExit(int code);
void SyscallMount(const char* mountPoint, const char* destination);
int SyscallOpen(const char *pathname, int flags);
int SyscallRead(Registers* registers); // TODO ssize_t, size_t
int SyscallWaitpid(Registers* registers); // TODO ssize_t, size_t

#endif
