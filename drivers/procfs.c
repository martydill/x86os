
#include <kernel.h>
#include <device.h>
#include <process.h>
#include <kernel_shared.h>
#include <process.h>

const char* PROC_FIELD_CMDLINE = "cmdline";

Device procFSDevice;

STATUS ProcFSRead(char* name, int numBytes) {
  Debug("Reading %s\n", name);

  char localName[255];
  strcpy(localName, name, strlen(name));

  char* part = strtok(localName, '/');
  part = strtok(NULL, '/');
  part = strtok(NULL, '/');
  int processId = atoi(part);

  Debug("For process %d\n", processId);
  ProcessList* node = ProcessGetProcessListNodeById(processId);
  if (node == NULL) {
    Debug("Could not find process\n");
    return S_FAIL;
  }
  Debug("Found process\n");
  Debug(node->Process->CommandLine);
  char* data = KMalloc(strlen(node->Process->CommandLine));
  strcpy(data, node->Process->CommandLine, strlen(node->Process->CommandLine));

  return data;
}
STATUS ProcFSOpen(char* name, int numBytes) {
  Debug("Opening %s\n", name);
  return S_OK;
}

STATUS ProcFSOpenDir(char* name, struct _DirImpl* dir) {
  dir->Count = 0;
  dir->Current = 0;

  ProcessList* processList = ProcessGetProcesses();
  while (processList != NULL && processList->Process != NULL) {
    struct dirent* currentEntry = &dir->dirents[dir->Count];
    currentEntry->st_mode = S_IFDIR;
    sprintf(sizeof(currentEntry->d_name), currentEntry->d_name, "%d",
            processList->Process->Id);
    // strcpy(dir->dirents[dir->Count].d_name, processList->Process->Name,
    // strlen(processList->Process->Name));
    Debug("Process: %s\n", dir->dirents[dir->Count].d_name);
    dir->Count++;
    processList = processList->Next;
  }

  return S_OK;
}

STATUS ProcFSStat(char* name, struct stat* statbuf) {
  // TODO finish implementing this
  ProcessList* processList = ProcessGetProcesses();
  while (processList != NULL && processList->Process != NULL) {
    statbuf->st_mode = S_IFDIR;
    processList = processList->Next;
  }
  return S_OK;
}

STATUS ProcFSInit(void) {
  procFSDevice.Name = "procfs";
  procFSDevice.Read = ProcFSRead;
  procFSDevice.OpenDir = ProcFSOpenDir;
  procFSDevice.Stat = ProcFSStat;
  procFSDevice.Open = ProcFSOpen;
  procFSDevice.Status = 0;
  procFSDevice.Status |= DEVICE_OPEN;
  procFSDevice.Status |= DEVICE_CAN_READ;
  DeviceRegister(&procFSDevice);

  return S_OK;
}

STATUS ProcFSDestroy(void) {
  DeviceUnregister(&procFSDevice);
  return S_OK;
}
