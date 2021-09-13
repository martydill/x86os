
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

// unistd.h

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// termios.h

#define NCCS 32

typedef int cc_t;
typedef int speed_t;
typedef int tcflag_t;

typedef struct termios {
  tcflag_t c_iflag;
  tcflag_t c_oflag;
  tcflag_t c_cflag;
  tcflag_t c_lflag;
  cc_t c_cc[NCCS];
};

#define TCSAFLUSH 0x03

#define BRKINT 0x01
#define ICRNL 0x02
#define IGNBRK 0x03
#define IGNCR 0x04
#define IGNPAR 0x05
#define INLCR 0x06
#define INPCK 0x07
#define ISTRIP 0x08
#define IXANY 0x09
#define IXOFF 0x0A
#define IXON 0x0B
#define PARMRK 0x0C

#define ECHO 1 << 1
#define ECHOE 1 << 2
#define ECHOK 1 << 3
#define ECHONL 1 << 4
#define ICANON 1 << 5
#define IEXTEN 1 << 6
#define ISIG 1 << 7
#define NOFLSH 1 << 8
#define TOSTOP 1 << 9

#define S_IFMT 0x01
#define S_IFBL 0x02
#define S_IFCHR 0x03
#define S_IFIFO 0x04
#define S_IFREG 0x05
#define S_IFDIR 0x06
#define S_IFLNK 0x07
#define S_IFSOCK 0x08

#define OPOST 1 << 1
#define ONLCR 1 << 2
#define OCRNL 1 << 3
#define ONOCR 1 << 4
#define ONLRET 1 << 5
#define OFILL 1 << 6

#define CSIZE 1 << 1
#define CS5 1 << 2
#define CS6 1 << 3
#define CS7 1 << 4
#define CS8 1 << 1
#define CSTOPB 1 << 5
#define CREAD 1 << 6
#define PARENB 1 << 7
#define PARODD 1 << 8
#define HUPCL 1 << 9
#define CLOCAL 1 << 10

#define VMIN 1
#define VTIME 255

// time.h
typedef int time_t;

typedef int pid_t;
typedef unsigned int ino_t;
typedef unsigned int size_t;
typedef int ssize_t;

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
  dev_t st_dev;
  ino_t st_ino;
  mode_t st_mode;
  nlink_t st_nlink;
  uid_t st_uid;
  gid_t st_gid;
  dev_t st_rdev;
  off_t st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
  blksize_t st_blksize;
  blkcnt_t st_blocks;
};

typedef struct dirent {
  ino_t d_ino;
  char d_name[255];

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

// errno.h
extern int errno;

// fcntl.h

#define O_CLOEXEC 1 << 1
#define O_CREAT 1 << 2
#define O_DIRECTORY 1 << 3
#define O_EXCL 1 << 4
#define O_NOCTTY 1 << 5
#define O_NOFOLLOW 1 << 6
#define O_TRUNC 1 << 7
#define O_TTY_INIT 1 << 8

// string.h
char* strtok(char* restrict s, const char delim);

#endif
