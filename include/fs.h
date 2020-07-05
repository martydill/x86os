
#ifndef FS_H
#define FS_H

#define FS_TYPE_FAT 1

typedef struct {
  BYTE Type;
  char* MountPoint;
  Device* Device;
} Filesystem;


STATUS FSInit();

#endif
