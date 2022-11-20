#define DEBUG 1

#include <kernel.h>
#include <kernel_shared.h>
#include <device.h>
#include <fs.h>

Filesystem* RootFS = NULL;
Filesystem* ProcFS = NULL;
Filesystem* NetFS = NULL;
Filesystem* NullFS = NULL;

Filesystem* FSMount(char* deviceName, char* mountPoint, BYTE type,
                    Filesystem* parent) {
  Device* device = GetDevice(deviceName);
  if (device == NULL) {
    Debug("Could not find device named %s\n", deviceName);
    return NULL;
  }

  Filesystem* fs = (Filesystem*)KMalloc(sizeof(Filesystem));
  if (fs == NULL) {
    return NULL;
  }
  Memset((BYTE*)fs, 0, sizeof(Filesystem));

  fs->Type = type;
  fs->Device = device;
  fs->MountPoint = (char*)KMalloc(strlen(mountPoint) + 1);
  if (fs->MountPoint == NULL) {
    return NULL;
  }

  Memcopy((BYTE*)fs->MountPoint, (BYTE*)mountPoint, strlen(mountPoint) + 1);

  if (parent != NULL) {
    Debug("Setting to child of %s\n", parent->Device->Name);
    parent->Children[parent->NumberOfChildren] = fs;
    parent->NumberOfChildren++;
  }
  return fs;
}

STATUS FSUnmount(char* mountPoint) { return S_OK; }

Filesystem* FSForPath(char* path) {
  Debug("Searching for path %s %s\n", path, RootFS->MountPoint);
  if (path == NULL) {
    return NULL;
  }
  Debug("%s %s\n", path, RootFS->MountPoint);
  if (!strcmp(path, RootFS->MountPoint)) {
    Debug("Found match\n");
    return RootFS;
  }
  Debug("No match, searching children\n");
  for (int i = 0; i < RootFS->NumberOfChildren; ++i) {
    Debug("Checking for child '%s'\n", RootFS->Children[i]->MountPoint);
    if (!strncmp(path, RootFS->Children[i]->MountPoint,
                 strlen(RootFS->Children[i]->MountPoint))) {
      Debug("Found match in children\n");
      return RootFS->Children[i];
    }
  }

  Debug("Did not find match, using root\n");
  return RootFS;
}

Device* FSDeviceForPath(char* path) {
  Filesystem* fs = FSForPath(path);
  if (fs != NULL) {
    return fs->Device;
  } else {
    return NULL;
  }
}

// Given an already populated dir object, check to see if we have any
// mounts at the same level. If we do, add direntries for them.
STATUS FSAddMountsToDir(const char* dirName, struct _DirImpl* dir) {
  Filesystem* fs = FSForPath(dirName);
  if (fs == NULL) {
    // No mounts here, not technically a failure..
    return S_OK;
  }

  // TODO: Handle case where we're not mounted at the root level
  for (int i = 0; i < fs->NumberOfChildren; ++i) {
    Filesystem* childFS = fs->Children[i];
    struct dirent* currentEntry = &dir->dirents[dir->Count];
    // Mount names start with / but we want to strip it here
    sprintf(strlen(childFS->MountPoint) - 1, currentEntry->d_name, "%s",
            childFS->MountPoint + 1);
    dir->dirents[dir->Count].st_mode = S_IFDIR;
    dir->Count = dir->Count + 1;
  }

  return S_OK;
}

STATUS FSInit() {
  RootFS = FSMount("floppy0", "/", FS_TYPE_FAT, NULL);
  Debug("Found rootfs %u %s\n", RootFS, RootFS->MountPoint);

  ProcFS = FSMount("procfs", "/proc", FS_TYPE_VIRTUAL, RootFS);
  Debug("Found procfs %u %s\n", ProcFS, ProcFS->MountPoint);

  NetFS = FSMount("netfs", "/net", FS_TYPE_VIRTUAL, RootFS);
  Debug("Found netfs%u %s\n", NetFS, NetFS->MountPoint);

  NullFS = FSMount("null", "/null", FS_TYPE_VIRTUAL, RootFS);
  Debug("Found null %u %s\n", NullFS, NullFS->MountPoint);

  return S_OK;
}
