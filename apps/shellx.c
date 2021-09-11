#include "kernel_shared.h"

int builtin_cd(const char* destination) {
  int err = chdir(destination);
  return err;
}

int main(int argc, char* argv[]) {
  char workingDirectory[255];

  KPrint("Starting shell\n");
  while (1) {

    if (!getcwd(workingDirectory, sizeof(workingDirectory))) {
      KPrint("Unable to get directory\n");
      return 1;
    }

    KPrint("\nlocalhost: ");
    KPrint(workingDirectory);
    KPrint(" $ ");

    char buf[255];

    int runInBackground = 0;
    int bytes = read(0, buf, 255);
    if (bytes > 0) {
      buf[bytes - 1] = 0;

      // Check for running in background with command&
      if (bytes > 2 && buf[bytes - 2] == '&') {
        runInBackground = 1;
        buf[bytes - 2] = 0; // Remove from actual command line
      }

      char binaryName[255];
      char* p = buf;
      char* q = binaryName;
      while (*p != ' ') {
        *q++ = *p++;
      }
      *q = 0;
      p++;

      // Check for builtins
      if (!strcmp(binaryName, "cd")) {
        builtin_cd(p);
        continue;
      }

      // If no builtins, try running command
      pid_t pid;
      if (posix_spawn(&pid, binaryName, 0, 0, buf, 0) == -1) {
        KPrint("Could not find ");
        KPrint(binaryName);
        continue;
      }

      if (!runInBackground) {
        // If we're running in background, do not wait
        waitpid(pid, 0, 0);
      }
    }
  }
}
