#include "kernel_shared.h"

int main(int argc, char* argv[])
{
  KPrint("Starting shell\n");
  while(1) {
    KPrint("$ ");
    char buf[255];
    int bytes = read(0, buf, 255);
    if(bytes > 0) {
      buf[bytes] = 0;
      // write(1, buf, bytes);
      pid_t pid;
      posix_spawn(&pid, buf, 0, 0, 0, 0);
    }
  }
}