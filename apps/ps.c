#include "kernel_shared.h"

int main(int argc, char* argv[]) {
  DIR* dir = opendir("/proc");
  if (dir == 0) {
    write(1, "Could not open dir\n", 20);
    return 1;
  }

  write(1, "pid\t\tcommand\n", 100);
  struct dirent* d;
  do {
    char fullPath[255];
    d = readdir(dir);
    if (d != NULL) {
      struct stat statbuf;
      int pid = atoi(d->d_name);
      sprintf(255, fullPath, "/proc/%s", d->d_name);
      if (stat(fullPath, &statbuf) == 0) {
        if (pid > 0 && statbuf.st_mode == S_IFDIR) {
          write(1, d->d_name, 20);

          sprintf(255, fullPath, "/proc/%s/cmdline", d->d_name);
          int fd = open(fullPath);
          if (fd > 0) {
            char fileBuf[255];
            int numBytes = read(fd, fileBuf, sizeof(fileBuf));
            if (numBytes > 0) {
              write(1, "        ", 8);
              write(1, fileBuf, numBytes);
            }
          }
        }
        write(1, "\n", 1);
      } else {
        write(1, "Stat failed\n", 20);
      }
    }
  } while (d != NULL);

  closedir(dir);
  return 0;
}