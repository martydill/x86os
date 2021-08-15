
#include <kernel.h>
#include <device.h>
#include <process.h>
#include <kernel_shared.h>
#include <process.h>

Device procFSDevice;

STATUS ProcFSRead(char* buffer, int numBytes) {
  Memcopy(buffer, "This is a procfs", 10);
  return S_OK;
}

STATUS ProcFSOpenDir(char* name, struct _DirImpl* dir) {
	dir->Count = 0;
	dir->Current = 0;

	ProcessList* processList = ProcessGetProcesses();
	while(processList != NULL && processList->Process != NULL) {
		struct dirent* currentEntry = &dir->dirents[dir->Count];
		currentEntry->st_mode = S_IFDIR;
		sprintf(sizeof(currentEntry->d_name), currentEntry->d_name, "%d", processList->Process->Id);
		//strcpy(dir->dirents[dir->Count].d_name, processList->Process->Name, strlen(processList->Process->Name));
		Debug("Process: %s\n", dir->dirents[dir->Count].d_name);
		dir->Count++;
		processList = processList->Next;
	}

	return S_OK;
}


STATUS ProcFSStat(char* name, struct stat * statbuf)  {
	// TODO finish implementing this
	ProcessList* processList = ProcessGetProcesses();
	while(processList != NULL && processList->Process != NULL) {
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


