

void _syscall(unsigned int sysCallNumber) {
   asm volatile("movl $0x05, %eax");
  __asm__ __volatile__("int $0x80");

}

void KPrint(const char* data){
  _syscall(0);
}