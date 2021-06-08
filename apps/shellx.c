#include "kernel_shared.h"

int builtin_cd(const char* destination) {
  int err = chdir(destination);
  return err;
}

int main(int argc, char* argv[]) {
  char workingDirectory[255];

  KPrint("Starting shell\n");
  while (1) {

    if(!getcwd(workingDirectory, sizeof(workingDirectory))) {
      KPrint("Unable to get directory\n");
      return 1;
    }

    KPrint("\nlocalhost: ");
    KPrint(workingDirectory);
    KPrint(" $ ");

    char buf[255];

    int bytes = read(0, buf, 255);
    if (bytes > 0) {
      buf[bytes - 1] = 0;

      char binaryName[255];
      char* p = buf;
      char* q = binaryName;
      while (*p != ' ') {
        *q++ = *p++;
      }
      *q = 0;
      p++; 

      // Check for builtins
      if(!strcmp(binaryName, "cd")) {
        builtin_cd(p);
        continue;
      }

      // If no builtins, try running command
      // TODO check for launch failures here
      pid_t pid;
      posix_spawn(&pid, binaryName, 0, 0, buf, 0);
      waitpid(pid, 0, 0);
    }
  }
}
