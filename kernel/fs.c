

#include <kernel.h>
#include <device.h>
#include <fs.h>

Filesystem* RootFS = NULL;
Filesystem* ProcFS = NULL;

int FSMount(char* deviceName, char* mountPoint, BYTE type, Filesystem* parent) {
  Device* device = GetDevice(deviceName);
  if (device == NULL) {
    Debug("Could not find device named %s\n", deviceName);
    return -1;
  }

  Filesystem* fs = (Filesystem*)KMalloc(sizeof(Filesystem));
  if (fs == NULL) {
    return -1;
  }
  Memset(fs, 0, sizeof(Filesystem));

  fs->Type = type;
  fs->Device = device;
  fs->MountPoint = KMalloc(strlen(mountPoint) + 1);
  if (fs->MountPoint == NULL) {
    return -1;
  }

  Memcopy(fs->MountPoint, mountPoint, strlen(mountPoint) + 1);

  if(parent != NULL) {
    Debug("Setting to child of %s\n", parent->Device->Name);
	  parent->Children[parent->NumberOfChildren] = fs;
	  parent->NumberOfChildren++;
  }
  return fs;
}

STATUS FSUnmount(char* mountPoint) { return S_OK; }


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
  Debug("No match, searching children\n");
  for(int i = 0; i < RootFS->NumberOfChildren; ++i) {
	  if(!strncmp(path, RootFS->Children[i]->MountPoint, strlen(RootFS->Children[i]->MountPoint))) {
		  Debug("Found match in children\n");
		  return RootFS->Children[i]->Device;
	  }
  }

  Debug("Did not find match, giving up\n");
  // TODO - search VFS tree
  return NULL;
}

STATUS FSInit() {
  RootFS = FSMount("floppy0", "/", FS_TYPE_FAT, NULL);
  Debug("Found rootfs %u %s\n", RootFS, RootFS->MountPoint);

  ProcFS = FSMount("procfs", "/proc", FS_TYPE_VIRTUAL, RootFS);
  Debug("Found procfs %u %s\n", ProcFS, ProcFS->MountPoint); 

  Device* procfs = FSDeviceForPath("/proc");
  Debug("Found device %s\n", procfs->Name);
  char buf[32];
  DeviceRead(procfs, buf, 32);
  Debug("Read from procfs: %s\n", buf);
  return S_OK;
}
