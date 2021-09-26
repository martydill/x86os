
#ifndef SYSCALL_H
#define SYSCALL_H
#include <kernel_shared.h>
#include <kernel.h>
#include <interrupt.h>

int SyscallKPrint(Registers* registers);
int SyscallExit(Registers* registers);
int SyscallMount(const char* mountPoint, const char* destination);
int SyscallOpen(Registers* registers);
int SyscallRead(Registers* registers); // TODO ssize_t, size_t
int SyscallPosixSpawn(Registers* registers);
int SyscallWaitpid(Registers* registers); // TODO ssize_t, size_t
int SyscallOpendir(Registers* registers);
int SyscallReaddir(Registers* registers);
int SyscallClosedir(Registers* registers);
int SyscallKill(Registers* registers);
int SyscallSleep(Registers* registers);
int SyscallStat(Registers* registers);
int SyscallChdir(Registers* registers);
int SyscallGetcwd(Registers* registers);

#endif
