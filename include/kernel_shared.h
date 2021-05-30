
#ifndef KERNEL_SHARED_H
#define KERNEL_SHARED_H

#define NULL (0)

#define SYSCALL_EXIT 0x00
#define SYSCALL_KPRINT 0x01
#define SYSCALL_MOUNT 0x02
#define SYSCALL_OPEN 0x03
#define SYSCALL_READ 0x04
#define SYSCALL_WRITE 0x05
#define SYSCALL_POSIX_SPAWN 0x06
#define SYSCALL_WAITPID 0x07
#define SYSCALL_OPENDIR 0x08
#define SYSCALL_READDIR 0x09
#define SYSCALL_SLEEP 0x0A

typedef int pid_t;
typedef unsigned int ino_t;

typedef struct dirent{
  ino_t  d_ino;
  char   d_name[255];
};

typedef struct _DirImpl {
 int Handle;  
 struct dirent dirents[255];
 int Current;
 int Count;
} DIR;


typedef struct {
  // todo
} posix_spawn_file_actions_t;

typedef struct {
  // todo
} posix_spawnattr_t;

#endif
