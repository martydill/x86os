
#include <kernel.h>
#include <device.h>
#include <process.h>
#include <kernel_shared.h>
#include <process.h>

const char* PROC_FIELD_CMDLINE = "cmdline";

Device procFSDevice;

char* ProcFSRead(char* name, int* bytesRead) {
  Debug("Reading %s\n", name);

  char localName[255];
  strcpy(localName, name, strlen(name));

  char* part = strtok(localName, '/');
  part = strtok(NULL, '/');
  part = strtok(NULL, '/');
  ProcessId processId = atoi(part);

  Debug("For process %d\n", processId);

  ProcessList* node = ProcessGetProcessListNodeById(processId);
  if (node == NULL) {
    Debug("Could not find process\n");
    return NULL;
  }
  Debug("Found process '%s'\n", node->Process->CommandLine);

  int sizeOfData = strlen(node->Process->CommandLine);
  *bytesRead = sizeOfData;
  char* data = KMalloc(sizeOfData);
  strcpy(data, node->Process->CommandLine, sizeOfData);

  return data;
}

STATUS ProcFSOpen(char* name, int numBytes) {
  Debug("Opening %s\n", name);
  return S_OK;
}

// Opens the procfs root dir. Needs to fill in the DirImpl with a bunch
// of entries corresponding to process ids.
STATUS ProcFSOpenRootDir(char* name, struct _DirImpl* dir) {
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

// Opens a procfs child process dir. Needs to fill in the DirImpl with a bunch
// of entries corresponding to the individual process fields.
STATUS ProcFSOpenChildDir(char* name, struct _DirImpl* dir) {
  // Skip past /proc/ part
  name = name + strlen("/proc/");
  ProcessId processId = (ProcessId)atoi(name);
  ProcessList* processList = ProcessGetProcessListNodeById(processId);
  if (processList == NULL || processList->Process == NULL) {
    return S_FAIL;
  }

  strcpy(dir->dirents[0].d_name, PROC_FIELD_CMDLINE,
         strlen(PROC_FIELD_CMDLINE) + 1);
  dir->dirents[0].st_mode = S_IFREG;
  dir->Count = 1;

  return S_OK;
}

STATUS ProcFSOpenDir(char* name, struct _DirImpl* dir) {
  dir->Count = 0;
  dir->Current = 0;
  Debug("ProcFSOpenDir: '%s'\n", name);
  // Root directory - fill with processes
  if (!strcmp(name, "/proc")) {
    return ProcFSOpenRootDir(name, dir);
  } else {
    return ProcFSOpenChildDir(name, dir);
  }
}

STATUS ProcFSStat(char* name, struct stat* statbuf) {
  // TODO finish implementing this
  ProcessList* processList = ProcessGetProcesses();
  while (processList != NULL && processList->Process != NULL) {
    // TODO set mode depending on whether we're looking in /proc or /proc/<id>
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
