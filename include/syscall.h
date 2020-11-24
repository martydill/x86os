
#ifndef SYSCALL_H 
#define SYSCALL_H

#define SYSCALL_EXIT 0x00
#define SYSCALL_KPRINT 0x01
#define SYSCALL_MOUNT 0x02
#define SYSCALL_OPEN 0x03
#define SYSCALL_READ 0x04
#define SYSCALL_WRITE 0x05


void SyscallKPrint(const char* data);
void SyscallExit(int code);
void SyscallMount(const char* mountPoint, const char* destination);
int SyscallOpen(const char *pathname, int flags);
int SyscallRead(Registers* registers); // TODO ssize_t, size_t

#endif
