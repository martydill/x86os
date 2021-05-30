
#include <kernel.h>
#include <device.h>
#include <floppy.h>
#include <console.h>
#include <process.h>

const char* promptChar = "# ";

void ProcessCommand(char* command) {
  char commandLine[64];
  strcpy(commandLine, command, 64);

  char buf[32];
  KPrint("\n");
  char* cmd = strtok(command, ' ');
  char* args = strtok(NULL, ' ');

  if (!strcmp(command, "clear")) {
    ConClearScreen();
    ConMoveCursor(0, 0);
    /*KResetCursor();*/
  } else if (!strcmp(command, "dev")) {
    DumpDeviceList();
  } else if (!strcmp(command, "remove")) {
    Device* device = GetDevice("Test");
    DeviceUnregister(device);
  } else if (!strcmp(command, "Test")) {
    Device device;
    device.Name = "/Keyboard";
    DeviceRegister(&device);
  } else if (!strcmp(command, "uptime")) {
    sprintf(32, buf, "Uptime: %ds", TimerGetUptime());
    KPrint(buf);
  } else if (!strcmp(command, "diskchanged")) {
    DiskChanged();
  } else if (!strcmp(command, "diskreset")) {
    DiskReset();
  } else if (!strcmp(command, "seek")) {
    static int seek = 0;
    FloppySeek(seek++);
  }
  // else if(!strcmp(cmd, "cat"))
  // {
  //     if(args != NULL) {
  //       //read(args);
  //     }
  // }
  else if (!strcmp(command, "ps")) {
    KPrint("Name\t\tID\t\tState\t\tCpu\t\tPriority\n");
    BYTE foreground;
    ProcessGetForegroundProcessId(&foreground);
    Process* processes = ProcessGetProcesses();
    for (BYTE i = 0; i < 255; ++i) {
      Process* p = &processes[i];
      if (p->State > 0) {
        KPrint("%s\t\t%d\t\t%d\t\t\t%d\t\t%s\n", p->Name, p->Id, p->State,
               p->CpuTicks, p->Id == foreground ? "(foreground)" : "");
      }
    }
  } else if (!strcmp(command, "ls")) {
    Device* device = FSDeviceForPath("/mnt/floppy0");
    if (device) {
      KPrint("Found device %u %s\n", device, device->Name);
    } else {
      KPrint("No device\n");
    }
  } else {
    // read(command, commandLine);
    // KPrint("'%s' is an unknown command", command);
  }

  KPrint("\n");
  KPrint(promptChar);
}

void ShellStart() {
  char buf[1];
  char command[64];
  char lastCommand[64];
  BYTE currentProcess;
  BYTE activeProcess;

  ProcessGetCurrentProcess(&currentProcess);

  int currentPos = 0;
  Memset(command, 0, sizeof(command));

  Device* device = GetDevice("Keyboard");

  KPrint(promptChar);

  Debug("Device: %d id: %d\n", device, currentProcess);

  while (1) {
    __asm__("hlt");
    ProcessGetForegroundProcessId(&activeProcess);
    if (activeProcess != currentProcess) {
      continue;
    }

    STATUS status = DeviceRead(device, buf, 1);
    if (status == S_OK) {
      if (buf[0] == '\n') {
        command[currentPos] = '\0';
        strcpy(lastCommand, command, strlen(command));
        ProcessCommand(command);
        Memset(command, 0, sizeof(command));
        currentPos = 0;
      } else if ((int)buf[0] == 72) {
        Debug("Command: %s\n", lastCommand);
        strcpy(command, lastCommand, strlen(lastCommand));
        currentPos = strlen(lastCommand);
      } else if ((int)buf[0] == 8) {
        // Backspace
        if (currentPos > 0) {
          command[--currentPos] = '\0';
          buf[0] = '\0';
          ConBackspace();
        } else {
          Debug("Can't backspace anymore, currentpos = %d\r\n", currentPos);
        }
      } else {
        KPrint("%c", buf[0]);
        command[currentPos++] = buf[0];
      }
    }
  }
}
