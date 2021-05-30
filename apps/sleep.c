#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    write(1, "Usage: sleep <seconds>\n", 22);
    return 1;
  }
  // TODO implement atoi
  sleep((int)argv[1][0] - (int)'0');
  return 0;
}