#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  char* path;

  if (argc == 1) {
    path = ".";
  }
  else {
    path = argv[1];
  }

  DIR* dir = opendir(path);
  if(dir == 0) {
    write(1, "Could not open dir\n", 20);
    return 1;
  }

  struct dirent *d;
  do {
    d = readdir(dir);
    if(d != NULL) {
        write(1, d->d_name, 20);
        write(1, "\n", 1);
    }
  }
  while(d != NULL);

  closedir(dir);
}