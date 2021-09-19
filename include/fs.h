
#ifndef FS_H
#define FS_H

#define FS_TYPE_FAT 1
#define FS_TYPE_VIRTUAL 2

typedef struct Filesystem {
  BYTE Type;
  char* MountPoint;
  Device* Device;
  struct Filesystem* Children[32];
  BYTE NumberOfChildren;
} Filesystem;

STATUS FSInit();

Filesystem* FSMount(char* deviceName, char* mountPoint, BYTE type,
                    Filesystem* parent);

#endif
