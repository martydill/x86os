

#include <kernel.h>
#include <device.h>
#include <fs.h>

Filesystem* RootFS = NULL;

int FSMount(char* deviceName, char* mountPoint, BYTE type) {
  Device* device = GetDevice(deviceName);
  if (device == NULL) {
    return -1;
  }

  Filesystem* fs = (Filesystem*)KMalloc(sizeof(Filesystem));
  if (fs == NULL) {
    return -1;
  }

  fs->Type = type;
  fs->Device = device;
  fs->MountPoint = KMalloc(strlen(mountPoint) + 1);
  if (fs->MountPoint == NULL) {
    return -1;
  }

  Memcopy(fs->MountPoint, mountPoint, strlen(mountPoint) + 1);
  return fs;
}

STATUS FSUnmount(char* mountPoint) { return S_OK; }

STATUS FSInit() {
  RootFS = FSMount("floppy0", "/mnt/floppy0", FS_TYPE_FAT);
  Debug("Found rootfs %u %s\n", RootFS, RootFS->MountPoint);
  return S_OK;
}

Device* FSDeviceForPath(char* path) {
  Debug("Searching for path %s %s\n", path, RootFS->MountPoint);
  if (path == NULL) {
    return NULL;
  }
  Debug("%s %s\n", path, RootFS->MountPoint);
  if (!strcmp(path, RootFS->MountPoint)) {
    Debug("Found match\n");
    return RootFS->Device;
  }
  Debug("No match");
  // TODO - search VFS tree
  return NULL;
}