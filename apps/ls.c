#include "kernel_shared.h"
#include "dirent.h"

int main(int argc, char* argv[]) {
  char* path;
  char workingDirectoryBuffer[255];

  if (argc == 1) {
    if (!getcwd(workingDirectoryBuffer, sizeof(workingDirectoryBuffer))) {
      write(1, "Could not get current working directory\n", 30);
      return 1;
    }
    path = workingDirectoryBuffer;
  } else {
    path = argv[1];
  }

  DIR* dir = opendir(path);
  if (dir == 0) {
    write(1, "Could not open dir\n", 20);
    return 1;
  }

  struct dirent* d;
  do {
    char fullPath[255];
    d = readdir(dir);
    if (d != NULL) {
      struct stat statbuf;
      sprintf(255, fullPath, "%s%s%s", path, (strlen(path) == 1 ? "" : "/"),
              d->d_name);
      // write(1, fullPath, 99);
      // strcpy(fullPath), d->d_name);
      if (stat(fullPath, &statbuf) == 0) {
        write(1, d->d_name, 20);
        if (statbuf.st_mode == S_IFDIR) {
          write(1, "/", 1);
        }
        write(1, "\n", 1);
      } else {
        // write(1, "Stat failed\n", 20);
      }
    }
  } while (d != NULL);

  closedir(dir);
  return 0;
}