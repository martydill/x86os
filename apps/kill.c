#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    write(1, "Usage: kill <pid>\n", 22);
    return 1;
  }

  pid_t pid = atoi(argv[1]);
  int sig = 1;
  if (kill(pid, sig) == -1) {
    write(1, "Failed to kill\n", 20);
    return 1;
  }

  return 0;
}