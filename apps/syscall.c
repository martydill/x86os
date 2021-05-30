
#include "kernel_shared.h"
#include "../include/stdarg.h";

extern int main(int argc, char* argv[]);

int _syscall(unsigned int sysCallNumber, unsigned int param1,
             unsigned int param2, unsigned int param3, unsigned int param4,
             unsigned int param5) {
  int result;
  __asm__ __volatile__("int $0x80"
                       : "=a"(result)
                       : "a"((long)sysCallNumber), "b"((long)param1),
                         "c"((long)param2), "d"((long)param3), "S"(param4),
                         "D"(param5)
                       : "memory");
  return result;
}

void KPrint(const char* data) { _syscall(SYSCALL_KPRINT, data, 0, 0, 0, 0); }

void _exit(int code) { _syscall(SYSCALL_EXIT, code, 0, 0, 0, 0); }

void Mount(const char* mountPoint, const char* destination) {
  _syscall(SYSCALL_MOUNT, mountPoint, destination, 0, 0, 0);
}

int open(const char* pathname, int flags) {
  return _syscall(SYSCALL_OPEN, pathname, flags, 0, 0, 0);
}

int read(int fd, void* buf, int count) {
  return _syscall(SYSCALL_READ, fd, buf, count, 0, 0);
} // TODO ssize_t, size_t

int write(int fd, void* buf, int count) {
  return _syscall(SYSCALL_WRITE, fd, buf, count, 0, 0);
} //

int __attribute__((noreturn)) _start2(int argc, char* argv[]) {
  int returnCode = main(argc, argv);
  _exit(returnCode);
  while (1) {
  }
}

// todo support all params
int posix_spawn(pid_t* restrict pid, const char* restrict path,
                const posix_spawn_file_actions_t* file_actions,
                const posix_spawnattr_t* restrict attrp,
                char* const argv[restrict], char* const envp[restrict]) {
  return _syscall(SYSCALL_POSIX_SPAWN, pid, path, attrp, argv, envp);
}

pid_t waitpid(pid_t pid, int* status_ptr, int options) {
  return _syscall(SYSCALL_WAITPID, pid, status_ptr, options, 0, 0);
}

DIR *opendir(const char *name){
  return _syscall(SYSCALL_OPENDIR, name, 0, 0, 0, 0);
}

struct dirent *readdir(DIR *dirp) {
  if(dirp->Current >= dirp->Count) {
    return NULL;
  }

  struct dirent* d = &dirp->dirents[dirp->Current];
  dirp->Current++;
  return d;
}

int closedir(DIR* dir){
  return _syscall(SYSCALL_CLOSEDIR, dir, 0, 0, 0, 0);
}

unsigned int sleep(unsigned int seconds) {
  return _syscall(SYSCALL_SLEEP, seconds, 0, 0, 0, 0);
}