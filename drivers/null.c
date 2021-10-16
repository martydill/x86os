
#include <kernel.h>
#include <device.h>
#include <process.h>
#include <kernel_shared.h>
#include <process.h>

Device nullDevice;

char* NullDeviceRead(char* name, int* bytesRead) {
  Debug("NullDeviceRead %s\n", name);
  // TODO how to pass size?
  *bytesRead = 1;
  char* data = KMalloc(1);
  Memcopy(data, 0, 1);
  return data;
}

STATUS NullDeviceOpen(char* name, int numBytes) {
  Debug("NulLDeviceOpen%s\n", name);
  return S_OK;
}

STATUS NullDeviceOpenDir(char* name, struct _DirImpl* dir) {
  Debug("NulLDeviceOpenDir %s\n", name);
  dir->Count = 0;
  dir->Current = 0;
  return S_OK;
}

STATUS NullDeviceStat(char* name, struct stat* statbuf) {
  // TODO finish implementing this

  return S_OK;
}

STATUS NullDeviceInit(void) {
  nullDevice.Name = "null";
  nullDevice.Read = NullDeviceRead;
  nullDevice.OpenDir = NullDeviceOpenDir;
  nullDevice.Stat = NullDeviceStat;
  nullDevice.Open = NullDeviceOpen;
  nullDevice.Status = 0;
  nullDevice.Status |= DEVICE_OPEN;
  nullDevice.Status |= DEVICE_CAN_READ;
  DeviceRegister(&nullDevice);

  return S_OK;
}

STATUS NullDeviceDestroy(void) {
  DeviceUnregister(&nullDevice);
  return S_OK;
}
