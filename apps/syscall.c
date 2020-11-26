
#include "kernel_shared.h"
#include "../include/stdarg.h";

extern int main(int argc, char* argv[]);

int _syscall(unsigned int sysCallNumber, unsigned int param1, unsigned int param2, unsigned int param3, unsigned int param4) {
 int result;
  __asm__ __volatile__("int $0x80"
  : "=a" (result)
  : "a" ((long)sysCallNumber),"b" ((long)param1),"c" ((long)param2), \
          "d" ((long)param3),"S" (param4) : "memory");
  return result;
}

void KPrint(const char* data){
  _syscall(SYSCALL_KPRINT, data, 0, 0, 0);
}

void _exit(int code) {
  _syscall(SYSCALL_EXIT, code, 0, 0, 0);
}

void Mount(const char* mountPoint, const char* destination) {
  _syscall(SYSCALL_MOUNT, mountPoint, destination, 0, 0);
}

int open(const char *pathname, int flags) {
  return _syscall(SYSCALL_OPEN, pathname, flags, 0, 0);
}

int read(int fd, void *buf, int count) {
  return _syscall(SYSCALL_READ, fd, buf, count, 0);
}  // TODO ssize_t, size_t

int write(int fd, void *buf, int count) {
  return _syscall(SYSCALL_WRITE, fd, buf, count, 0);
}  // 

int __attribute__((noreturn)) _start2(int argc, char* argv[]) {
  int returnCode = main(argc, argv);
  _exit(returnCode);
  while(1){}
}