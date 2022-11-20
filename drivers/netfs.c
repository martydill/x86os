#define DEBUG 1
#include <kernel.h>
#include <device.h>
#include <kernel_shared.h>

Device netFSDevice;

char* NetFSRead(char* name, int* bytesRead) {
  Debug("Reading %s\n", name);

  char localName[255];
  strcpy(localName, name, strlen(name));

  char* part = strtok(localName, '/');
  part = strtok(NULL, '/');
  part = strtok(NULL, '/');
}

STATUS NetFSOpen(char* name, int numBytes) {
  Debug("Opening %s\n", name);
  return S_OK;
}

// Opens the netfs root dir. Needs to fill in the DirImpl with a bunch
// of entries corresponding to protocols.
STATUS NetFSOpenRootDir(char* name, struct _DirImpl* dir) {
  // Add entry for arp
  struct dirent* currentEntry = &dir->dirents[dir->Count];
  currentEntry->st_mode = S_IFDIR;
  sprintf(sizeof(currentEntry->d_name), currentEntry->d_name, "arp");
  dir->Count++;
  // Add future protocols here
  return S_OK;
}

// Opens a netfs protocol child dir.
// This will make a network request to the given protocl with the given params.
STATUS NetFSOpenChildDir(char* name, struct _DirImpl* dir) {
  Debug("Open child dir %s\n", name);
  // Skip past /net/ part
  char* protocolString = PathSkipFirstComponent(name) + 1;
  if (!strcmp(protocolString, "arp")) {
    Debug("Reading arp\n");
  }

  return S_OK;
}

STATUS NetFSOpenDir(char* name, struct _DirImpl* dir) {
  dir->Count = 0;
  dir->Current = 0;
  Debug("NetFSOpenDir: '%s'\n", name);
  // Root directory - fill with protocols
  // TODO make this nicer
  if (!strcmp(name, "/net") || !strcmp(name, "net")) {
    return NetFSOpenRootDir(name, dir);
  } else {
    return NetFSOpenChildDir(name, dir);
  }
}

STATUS NetFSStat(char* name, struct stat* statbuf) {
  int depth = PathGetDepth(name);
  if (depth == 1) {
    // depth = 1, we're fetching the root /net dir, so
    // it's a directory
    statbuf->st_mode = S_IFDIR;
  } else if (depth == 2) {
    // depth = 2, we're fetching the list of protocols, so
    // each record is a directory
    statbuf->st_mode = S_IFDIR;
  } else {
    // Otherwise, it's a file
    statbuf->st_mode = S_IFREG;
  }
  Debug("Stat, path='%s', depth = %d, mode = %d\n", name, depth,
        statbuf->st_mode);
  return S_OK;
}

STATUS NetFSInit(void) {
  netFSDevice.Name = "netfs";
  netFSDevice.Read = NetFSRead;
  netFSDevice.OpenDir = NetFSOpenDir;
  netFSDevice.Stat = NetFSStat;
  netFSDevice.Open = NetFSOpen;
  netFSDevice.Status = 0;
  netFSDevice.Status |= DEVICE_OPEN;
  netFSDevice.Status |= DEVICE_CAN_READ;
  DeviceRegister(&netFSDevice);

  return S_OK;
}

STATUS NetFSDestroy(void) {
  DeviceUnregister(&netFSDevice);
  return S_OK;
}
