
#include "syscall.h"
#include "../include/stdarg.h";

extern int main(int argc, char* argv[]);

void _syscall(unsigned int sysCallNumber, unsigned int param1, unsigned int param2, unsigned int param3, unsigned int param4) {
  __asm__ __volatile__("int $0x80"
  :: "a" ((long)sysCallNumber),"b" ((long)param1),"c" ((long)param2), \
          "d" ((long)param3),"S" (param4) : "memory");
}

void KPrint(const char* data){
  _syscall(SYSCALL_KPRINT, data, 0, 0, 0);
}

void Exit(int code) {
  _syscall(SYSCALL_EXIT, 0, 0, 0, 0);
}

void Mount(const char* mountPoint, const char* destination) {
  _syscall(SYSCALL_MOUNT, mountPoint, destination, 0, 0);
}

int __attribute__((noreturn)) _start2(int argc, char* argv[]) {
  int returnCode = main(argc, argv);
  Exit(returnCode);
  while(1){}
}