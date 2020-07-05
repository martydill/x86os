

#include <kernel.h>
#include <device.h>
#include <fs.h>

Filesystem* RootFS = NULL;

FSMount(Device* device, char* mountPoint, BYTE type)
{
  Filesystem* fs = (Filesystem*)KMalloc(sizeof(Filesystem));
  fs->Type = type;
  fs->Device = device;
  fs->MountPoint = KMalloc(Strlen(mountPoint));
  Memcopy(&fs->MountPoint, mountPoint, Strlen(mountPoint));
  return fs;
}

STATUS FSUnmount(char* mountPoint)
{
  return S_OK;
}

STATUS FSInit()
{
  Device* floppyDevice = GetDevice("Floppy");
  RootFS = FSMount(floppyDevice, "/", FS_TYPE_FAT);
  return S_OK;
}