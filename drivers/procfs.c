
#include <kernel.h>
#include <device.h>
#include <process.h>
#include <kernel_shared.h>
#include <process.h>

#define PROC_FIELD_CMDLINE "cmdline"
#define PROC_FIELD_ID "id"
#define PROC_FIELD_STATUS "status"

const char* const ProcFieldNames[] = {PROC_FIELD_ID, PROC_FIELD_CMDLINE,
                                      PROC_FIELD_STATUS};

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
  Process* process = node->Process;

  Debug("Found process '%s'\n", process->CommandLine);

  // todo parse last part of path instead of doing this
  char* field = PathSkipFirstComponent(PathSkipFirstComponent(name));
  if (*field == '/') {
    field++;
  }

  if (!strcmp(field, PROC_FIELD_CMDLINE)) {
    int sizeOfData = strlen(process->CommandLine);
    *bytesRead = sizeOfData;
    char* data = KMalloc(sizeOfData);
    strcpy(data, process->CommandLine, sizeOfData);
    return data;
  } else if (!strcmp(field, PROC_FIELD_ID)) {
    // todo use better method of determining size
    char* data = KMalloc(4);
    sprintf(4, data, "%d", process->Id);
    *bytesRead = 4;
    return data;
  } else if (!strcmp(field, PROC_FIELD_STATUS)) {
    // todo use better method for determining size
    char* data = KMalloc(20);
    const char* status = ProcessStatusToString(process);
    strcpy(data, status, strlen(status));
    *bytesRead = strlen(status);
    return data;
  }
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
  for (int i = 0; i < sizeof(ProcFieldNames) / sizeof(const char*); ++i) {
    strcpy(dir->dirents[i].d_name, ProcFieldNames[i],
           strlen(ProcFieldNames[i]) + 1);
    dir->dirents[i].st_mode = S_IFREG;
    dir->Count = dir->Count + 1;
  }

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
  int depth = PathGetDepth(name);
  if (depth == 1) {
    // depth = 1, we're fetching the root /proc dir, so
    // it's a directory
    statbuf->st_mode = S_IFDIR;
  } else if (depth == 2) {
    // depth = 2, we're fetching the list of processes, so
    // each record is a directory
    statbuf->st_mode = S_IFDIR;
  } else {
    // Otherwise, it's a file
    statbuf->st_mode = S_IFREG;
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
