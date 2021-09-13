
#include "kernel_shared.h"
#include "../include/stdarg.h"

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

void KPrint(const char* data) { _syscall(SYSCALL_KPRINT,  (unsigned int)data, 0, 0, 0, 0); }

void _exit(int code) { _syscall(SYSCALL_EXIT,  (unsigned int)code, 0, 0, 0, 0); }

void Mount(const char* mountPoint, const char* destination) {
  _syscall(SYSCALL_MOUNT,  (unsigned int)mountPoint,  (unsigned int)destination, 0, 0, 0);
}

int open(const char* pathname, int flags) {
  return _syscall(SYSCALL_OPEN,  (unsigned int)pathname,  (unsigned int)flags, 0, 0, 0);
}

int read(int fd, void* buf, int count) {
  return _syscall(SYSCALL_READ,  (unsigned int)fd,  (unsigned int)buf,  (unsigned int)count, 0, 0);
} // TODO ssize_t, size_t

int write(int fd, void* buf, int count) {
  return _syscall(SYSCALL_WRITE,  (unsigned int)fd,  (unsigned int)buf,  (unsigned int)count, 0, 0);
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
  return _syscall(SYSCALL_POSIX_SPAWN,  (unsigned int)pid,  (unsigned int)path,  (unsigned int)attrp,  (unsigned int)argv,  (unsigned int)envp);
}

pid_t waitpid(pid_t pid, int* status_ptr, int options) {
  return _syscall(SYSCALL_WAITPID,  (unsigned int)pid,  (unsigned int)status_ptr,  (unsigned int)options, 0, 0);
}

DIR* opendir(const char* name) {
  return (DIR*)_syscall(SYSCALL_OPENDIR, (unsigned int)name, 0, 0, 0, 0);
}

struct dirent* readdir(DIR* dirp) {
  if (dirp->Current >= dirp->Count) {
    return NULL;
  }

  struct dirent* d = &dirp->dirents[dirp->Current];
  dirp->Current++;
  return d;
}

int closedir(DIR* dir) { return _syscall(SYSCALL_CLOSEDIR,  (unsigned int)dir, 0, 0, 0, 0); }

int chdir(const char* path) {
  return _syscall(SYSCALL_CHDIR,  (unsigned int)path, 0, 0, 0, 0);
}

char* getcwd(char* buf, size_t size) {
  return (char*)_syscall(SYSCALL_GETCWD,  (unsigned int)buf,  (unsigned int)size, 0, 0, 0);
}

unsigned int sleep(unsigned int seconds) {
  return _syscall(SYSCALL_SLEEP, seconds, 0, 0, 0, 0);
}

int kill(pid_t pid, int sig) {
  return _syscall(SYSCALL_KILL,  (unsigned int)pid,  (unsigned int)sig, 0, 0, 0);
}

int stat(const char* restrict path, struct stat* restrict buf) {
  return _syscall(SYSCALL_STAT, (unsigned int)path,  (unsigned int)buf, 0, 0, 0);
}

int fstat(int fildes, struct stat* buf) {
  return _syscall(SYSCALL_FSTAT, (unsigned int)fildes, (unsigned int)buf, 0, 0, 0);
}