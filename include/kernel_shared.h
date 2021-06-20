
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
#define SYSCALL_CLOSEDIR 0x0A
#define SYSCALL_CHDIR 0x0B
#define SYSCALL_GETCWD 0x0C
#define SYSCALL_SLEEP 0x0D
#define SYSCALL_KILL 0x0E
#define SYSCALL_STAT 0x0F
#define SYSCALL_FSTAT 0x10


#define S_IFMT 0x01
#define S_IFBL 0x02
#define S_IFCHR 0x03
#define S_IFIFO 0x04
#define S_IFREG 0x05
#define S_IFDIR 0x06
#define S_IFLNK 0x07
#define S_IFSOCK 0x08

typedef int pid_t;
typedef unsigned int ino_t;
typedef unsigned int size_t;

typedef int dev_t;
typedef int mode_t;
typedef int nlink_t;
typedef int uid_t;
typedef int gid_t;
typedef int dev_t;
typedef int off_t;
typedef int time_t;
typedef int blksize_t;
typedef int blkcnt_t;

typedef struct stat {
  dev_t     st_dev;
  ino_t     st_ino;
  mode_t    st_mode;
  nlink_t   st_nlink;
  uid_t     st_uid;
  gid_t     st_gid;
  dev_t     st_rdev;
  off_t     st_size;
  time_t    st_atime;
  time_t    st_mtime;
  time_t    st_ctime;
  blksize_t st_blksize;
  blkcnt_t  st_blocks;
};

typedef struct dirent{
  ino_t  d_ino;
  char   d_name[255];

  // Non-posix fields
  off_t st_size;
  mode_t st_mode;
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
