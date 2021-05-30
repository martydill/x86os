#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    write(1, "Usage: cd <path>\n", 22);
    return 1;
  }

  if(!chdir(argv[1])) {
    write(1, "Could not change to dir\n", 20);
    return 1;
  }

  return 0;
}