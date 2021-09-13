
#ifndef DEVICE_H
#define DEVICE_H

#define DEVICE_OPEN (1 << 0)
#define DEVICE_CAN_READ (1 << 1)
#define DEVICE_CAN_WRITE (1 << 2)

#define MAX_DEVICE_NAME 32

struct stat;
struct _DirImpl;

// void* FloppyReadDirectory(char* name, struct _DirImpl* dirimpl) {
typedef STATUS (*DeviceFunc)(char*, int);
typedef char* (*DeviceReadFunc)(char*, int*);
typedef STATUS (*DeviceStatFunc)(char*, struct stat* statbuf);
typedef STATUS (*DeviceOpenDirFunc)(char* name, struct _DirImpl* dir);

struct Device_S;

typedef struct DeviceList_S {
  struct Device_S* Device;
  struct DeviceList_S* Next;
  struct DeviceList_S* Previous;
} DeviceList;

typedef struct Device_S {
  char* Name;
  DWORD Status;
  DeviceReadFunc Read;
  DeviceFunc Write;
  DeviceFunc Open;
  DeviceFunc Close;
  DeviceOpenDirFunc OpenDir;
  DeviceStatFunc Stat;
  DeviceList* Children;
} Device;

STATUS DeviceRegister(Device* device);

STATUS DeviceUnregister(Device* device);

#endif
