#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  char* path;
  char workingDirectoryBuffer[255];

  if (argc == 1) {
    if(!getcwd(workingDirectoryBuffer, sizeof(workingDirectoryBuffer))) {
      write(1, "Could not get current working directory\n", 30);
      return 1;
    }
    path = workingDirectoryBuffer;
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
  return 0;
}