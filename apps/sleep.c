#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    write(1, "Usage: sleep <seconds>\n", 22);
    return 1;
  }
  sleep(atoi(argv[1]));
  return 0;
}