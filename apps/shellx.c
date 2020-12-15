#include "kernel_shared.h"

int main(int argc, char* argv[])
{
  KPrint("Starting shell\n");
  while(1) {
    KPrint("$ ");
    char buf[255];
    int bytes = read(0, buf, 255);
    if(bytes > 0) {
      buf[bytes - 1] = 0;

      char binaryName[255];
      char* p = buf;
      char* q = binaryName;
      while(*p != ' ') {
        *q++ = *p++;
      }
      *q = 0;

      pid_t pid;
      posix_spawn(&pid, binaryName, 0, 0, buf, 0);
    }
  }
}
