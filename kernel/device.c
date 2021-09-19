
#include <kernel.h>
#include <device.h>

const char* RootDeviceName = "Dev";

Device Root;
Device* devices[256];
int numDevices = 0;

DeviceList* FindDeviceListEntry(Device* device) {
  Assert(device != NULL);
  DeviceList* deviceList = Root.Children;
  Assert(deviceList != NULL);

  while (deviceList != NULL && deviceList->Device != device) {
    deviceList = deviceList->Next;
  }

  return deviceList;
}

/* Initialize device handler */
STATUS DeviceInit() {
  Root.Name = RootDeviceName;
  Root.Read = NULL;
  Root.Write = NULL;
  Root.Open = NULL;
  Root.Close = NULL;
  Root.Status = 0;
  Root.Children = NULL;
  return S_OK;
}

/* Reads data from the specified device into the supplied buffer */
STATUS DeviceRead(Device* device, char* buffer, int numBytes) {
  Assert(device != NULL);
  Assert(buffer != NULL);

  /* Make sure the device is open */
  if (!(device->Status & DEVICE_OPEN)) {
    Debug("Unable to read from %s, device is not open  %d %d\r\n", device->Name,
          device, device->Status);
    return S_FAIL;
  }

  /* Make sure it's ready for a read */
  if (!(device->Status & DEVICE_CAN_READ)) {
    Debug("Unable to read from %s, device is not ready for a read\r\n",
          device->Name);
    return S_FAIL;
  }

  /* Make sure it has a read handler */
  if (device->Read == NULL) {
    Debug("Unable to read from %s, device does not have a read handler\r\n",
          device->Name);
    return S_FAIL;
  }

  return device->Read(buffer, numBytes);
}

/* Writes the specified data to the device */
STATUS DeviceWrite(Device* device, char* buffer, int numBytes) {
  Assert(device != NULL);
  Assert(buffer != NULL);

  /* Make sure the device is open */
  if (!(device->Status & DEVICE_OPEN))
    return S_FAIL;

  /* Make sure it's ready for a write */
  if (!(device->Status & DEVICE_CAN_WRITE))
    return S_FAIL;

  /* Make sure it has a write handler */
  if (device->Write == NULL)
    return S_FAIL;

  return device->Write(buffer, numBytes);
}

/* Adds the specified device to the device list */
void AddToList(Device* device) {
  DeviceList* prev = NULL;
  DeviceList* deviceList = Root.Children;

  if (Root.Children == NULL) {
    Root.Children = (DeviceList*)KMalloc(sizeof(DeviceList));
    Root.Children->Device = device;
    Root.Children->Next = NULL;
    return;
  }

  while (deviceList != NULL) {
    prev = deviceList;
    deviceList = deviceList->Next;
  }

  deviceList = (DeviceList*)KMalloc(sizeof(DeviceList));
  deviceList->Device = device;
  deviceList->Next = NULL;

  if (prev != NULL) {
    prev->Next = deviceList;
    deviceList->Previous = prev;
  }
}

/* Removes the specified device from the device list */
STATUS RemoveFromList(Device* device) {
  DeviceList* listEntry = FindDeviceListEntry(device);
  if (listEntry == NULL)
    return S_FAIL;

  if (listEntry->Previous != NULL)
    listEntry->Previous->Next = listEntry->Next;
  if (listEntry->Next != NULL)
    listEntry->Next->Previous = listEntry->Previous;

  return S_OK;
}

/* Registers the given device in the device tree */
STATUS DeviceRegister(Device* device) {
  Assert(device != NULL);

  AddToList(device);
  return S_OK;
}

/* Unregisters the given device */
STATUS DeviceUnregister(Device* device) {
  Assert(device != NULL);

  RemoveFromList(device);
  return S_OK;
}

/* Returns the device with the specified name */
Device* GetDevice(const char* deviceName) {
  Assert(deviceName != NULL);
  DeviceList* list = Root.Children;

  while (list != NULL) {
    Device* temp = list->Device;
    if (temp != NULL) {
      if (!strcmp(deviceName, temp->Name))
        return temp;
    }
    list = list->Next;
  }
  return NULL;
}

void DumpDeviceList(void) {
  KPrint("Device tree:\n");
  KPrint("/%s\n", Root.Name);
  DeviceList* list = Root.Children;

  while (list != NULL) {
    Device* temp = list->Device;
    if (temp != NULL)
      KPrint("  /%s\n", temp->Name);

    list = list->Next;
  }
}
