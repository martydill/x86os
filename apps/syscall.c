

extern int main();

void _syscall(unsigned int sysCallNumber, unsigned int param1, unsigned int param2, unsigned int param3, unsigned int param4) {
  __asm__ __volatile__("int $0x80"
  :: "a" ((long)sysCallNumber),"b" ((long)param1),"c" ((long)param2), \
          "d" ((long)param3),"S" (param4) : "memory");
}

void KPrint(const char* data){
  _syscall(1, data, 0, 0, 0);
}

void Exit(int code) {
  _syscall(0, 0, 0, 0, 0);
}

int __attribute__((noreturn)) _start() {
  int returnCode = main();
  Exit(returnCode);
  while(1){}
}