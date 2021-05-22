
#ifndef KERNEL_SHARED_H
#define KERNEL_SHARED_H

#define SYSCALL_EXIT 0x00
#define SYSCALL_KPRINT 0x01
#define SYSCALL_MOUNT 0x02
#define SYSCALL_OPEN 0x03
#define SYSCALL_READ 0x04
#define SYSCALL_WRITE 0x05
#define SYSCALL_POSIX_SPAWN 0x06
#define SYSCALL_WAITPID 0x07

typedef int pid_t;

typedef struct {
  // todo
} posix_spawn_file_actions_t;

typedef struct {
  // todo
} posix_spawnattr_t;

#endif
