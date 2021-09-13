
/*
 * Floppy.c
 * Floppy disk driver
 * References:
 *  http://www.isdaman.com/alsos/hardware/fdc/floppy.htm
 */
#include <kernel_shared.h>
#include <kernel.h>
#include <floppy.h>
#include <io.h>
#include <interrupt.h>
#include <device.h>
#include <fat.h>
#include <dma.h>
#include <elf.h>
#include <process.h>

extern WORD FATSectorForCluster(FAT12BootSector* bs, WORD cluster);

const char* floppyTypeString[] = {"No floppy drive", "360k 5.25\"",
                                  "1.2m 5.25\"",     "720k 3.5\"",
                                  "1.44m 3.5\"",     "2.88m 3.5\""};

enum FloppyCommands {
  READ_SECTOR = 0xe6,
  SEEK_TRACK = 0x0f,
  SENSE_INTERRUPT = 0x08, /* or 'check interrupt status */
  SPECIFY = 0x03,
  RECALIBRATE = 0x07
};

enum FloppyRegisters {
  BASE = 0x3f0,
  STATUSA = 0x3f0,
  STATUSB = 0x3f1,
  DOR = 0x3f2,
  MSR = 0x3f4,
  DATARATE = 0x3f4,
  DATA = 0x3f5,
  DIRR = 0x3f7,
  CCR = 0x3f7
};

enum DORFlags {
  DR0 = 1 << 0,
  DR1 = 1 << 1,
  REST = 1 << 2,
  DMA = 1 << 3,
  MOTA = 1 << 4,
  MOTB = 1 << 5,
  MOTC = 1 << 6,
  MOTD = 1 << 7
};

enum MSRFlags {
  ACTA = 1 << 0,
  ACTB = 1 << 1,
  ACTC = 1 << 2,
  ACTD = 1 << 3,
  BUSY = 1 << 4,
  NDMA = 1 << 5,
  DIO = 1 << 6,
  MRQ = 1 << 7
};

enum DIRFlags { RAT0 = 1 << 1, RAT1 = 1 << 2, CHAN = 1 << 7 };

enum DataTransferRate { RATE_500kbit, RATE_300kbit, RATE_250kbit, RATE_1mbit };

typedef struct FloppyDrive_S {
  BYTE dor;
  BYTE msr;
  BYTE present;
  int track;
  int HeadsPerCylinder;
  int SectorsPerTrack;
  Device device;
} FloppyDrive;

FloppyDrive drives[2];
volatile int intcount = 0;
void GetDriveStatus(FloppyDrive* drive);
void sleep(void) {
  int ticks = TimerGetTicks();
  while (TimerGetTicks() - ticks < 10) {
  }
}

/* Waits for the MRQ bit of the MSR to be set */
void WaitForReady(FloppyDrive* drive) {
  while (1) {
    GetDriveStatus(drive);

    if (drive->msr & MRQ)
      break;
  }
}

/* Waits for the MRQ bit of the MSR to be set */
void WaitForSeek(FloppyDrive* drive) {
  while (1) {
    GetDriveStatus(drive);

    Debug("Waiting for seek\n");
    if (!(drive->msr & ACTA)) {
      Debug("Done seek\n");
      break;
    }
  }
}

void WriteByte(BYTE byte) {
  volatile unsigned msr;
  unsigned tmo;

  WaitForReady(&drives[0]);
  for (tmo = 0; tmo < 128; tmo++) {
    msr = IoReadPortByte(MSR);
    if ((msr & 0xC0) == 0x80) {
      IoWritePortByte(DATA, byte);
      return;
    }
    IoReadPortByte(0x80); /* delay */
  }
  return; /* write timeout */
}

BYTE ReadByte() {
  volatile unsigned msr;
  unsigned short timeout;

  for (timeout = 0; timeout < 128; timeout++) {
    msr = IoReadPortByte(MSR);
    if ((msr & 0xD0) == 0xD0)
      return IoReadPortByte(DATA);
    IoReadPortByte(0x80); /* delay */
  }

  return 0; /* read timeout */
}

volatile int expectingInterrupt = 0;

/* IRQ 6 handler */
void FloppyHandler(Registers* registers) {
  Debug("Got floppy interrupt\n");
  expectingInterrupt = 1;
}

/* Starts the motor and waits a while for it to turn on */
void StartMotor(FloppyDrive* drive) {
  Assert(drive != NULL);
  drive->dor |= MOTA;
  IoWritePortByte(DOR, drive->dor);
  sleep();
}
void WaitForInterrupt();
void DiskRecalibrate() {
  WriteByte(RECALIBRATE);
  WriteByte(0);

  WaitForInterrupt();

  WriteByte(SENSE_INTERRUPT);
  ReadByte();
  ReadByte();
}

void DiskReset(FloppyDrive* drive) {
  Assert(drive != NULL);

  drive->dor = 0;
  IoWritePortByte(DOR, drive->dor);

  drive->dor |= DMA;
  drive->dor |= REST;
  drive->dor |= MOTA;
  IoWritePortByte(DOR, drive->dor);

  WaitForSeek(drive);
  // WaitForInterrupt();
  int i = 0;
  for (i = 0; i < 4; ++i) {
    WriteByte(SENSE_INTERRUPT);
    ReadByte();
    ReadByte();
  }

  IoWritePortByte(CCR, 0);
  WriteByte(SPECIFY);
  WriteByte(0xdf);
  WriteByte(0x02);
}

void GetDriveStatus(FloppyDrive* drive) {
  BYTE b = IoReadPortByte(MSR);
  drive->msr = b;
}

BOOL DiskChanged() {
  BYTE b = IoReadPortByte(DIRR);
  if (b & CHAN)
    return TRUE;

  return FALSE;
}

void WaitForReady2() {
  int c = 0;

  while ((c < 7) && (IoReadPortByte(MSR) & 0x10)) {
    ReadByte();
    c++;
  }
}

/* busy waits until we receive an interrupt */
void WaitForInterrupt() {
  Debug("Waiting for interrupt\n");
  while (!expectingInterrupt) {
  }
  expectingInterrupt = 0;
  Debug("Got interrupt\n");
}

/* Seeks to the given track */
STATUS FloppySeek(unsigned int track) {
  if (drives[0].track == track) {
    Debug("Already on track %u\n", track);
    return S_OK;
  }

  DiskReset(&drives[0]);
  Debug("Seeking to track %u ", track);
  WriteByte(SEEK_TRACK);
  WriteByte(0);
  WriteByte(track);
  WaitForSeek(&drives[0]);
  // WaitForInterrupt();
  // WaitForSeek(&drives[0]);
  // sleep();
  Debug("Done wait");

  /* Ensure that the seek was successful */
  WriteByte(SENSE_INTERRUPT);
  BYTE b = ReadByte();
  BYTE b2 = ReadByte();

  Debug("Now on track %d, sr0=%d\n", (int)b2, (int)b);
  if (b2 != track) {
    /* fixme: reset? */
    KPrint("Seek failed!\n");
    return S_FAIL;
  }

  drives[0].track = track;
  Debug("Done seek\n");
  return S_OK;
}

int dir = 0;
extern STATUS FATReadFile(BYTE* buffer, FAT12BootSector* bs, BYTE* fat,
                          WORD cluster);

extern FAT12BootSector s;

/* Reads the given sector into the supplied buffer */
STATUS FloppyReadSector(int sector, char* buffer) {
  Assert(buffer != NULL);
  int LBA = sector;

  int HPC = 2;
  int SPT = 18;
  int CYL = LBA / (HPC * SPT);
  int TEMP = LBA % (HPC * SPT);
  int HEAD = TEMP / SPT;
  int SECT = TEMP % SPT + 1;

  /*	KPrint("LBA: %d  CYL: %d  HEAD: %d  SECT: %d\n", LBA, CYL, HEAD,
   * SECT);*/

  if (DiskChanged()) {
    KPrint("Disk changed\n");
    return S_FAIL;
  }
  FloppySeek(CYL);

  IoWritePortByte(CCR, 0);

  DmaFloppyRead();
  WriteByte(READ_SECTOR);
  WriteByte((HEAD << 2));
  WriteByte(CYL);
  WriteByte(HEAD);
  WriteByte(SECT);
  WriteByte(2);
  WriteByte(18);
  WriteByte(0x1b);
  WriteByte(0xff);

  WaitForReady(&drives[0]);
  Debug("Waiting in read\n");
  WaitForSeek(&drives[0]);
  Debug("Done wait in read\n");

  /* clear up the fifo queue */
  int z = 0;
  for (z = 0; z < 7; ++z)
    ReadByte();

  Memcopy(buffer, FLOPPY_DMA_ADDRESS, FLOPPY_DMA_BUFFER_SIZE);
  // int i;
  // unsigned char* blah = (unsigned char*)0x0000;
  // for(i = 0; i < 512; ++i)
  // {
  //     buffer[i] = blah[i];
  // }
  // Debug(buffer);
  if (dir == 0) {
    dir = FATParseBootSector(buffer);
    // BYTE buffer[90 * 1024];
    // FATReadFile(&s, buffer, fat);
  }

  /* fixme: check for errors */
  /* fixme: turn off motor */
  /* fixme: use memcopy */
  return S_OK;
}

WORD FATGetNextCluster(BYTE* fat, WORD cluster);

STATUS FloppyStat(char* name, struct stat* statbuf) {
  // TODO figure out where / should be stripped out
  name = name + 1;
  Process* active = ProcessGetActiveProcess();
  struct _DirImpl* dir = KMallocInProcess(active, sizeof(struct _DirImpl));
  FloppyReadDirectory("/", dir);
  for (int i = 0; i < dir->Count; ++i) {
    if (!strcmp(name, dir->dirents[i].d_name)) {
      // TODO fill in other members of statbuf
      statbuf->st_mode = dir->dirents[i].st_mode;
      statbuf->st_size = dir->dirents[i].st_size;
      return S_OK;
    }
  }

  return S_FAIL;
}

STATUS FloppyOpen(char* name, int bytes) { return S_OK; }

STATUS FloppyReadDirectory(char* name, struct _DirImpl* dirimpl) {
  int count;
  BYTE buf[512];
  FloppyReadSector(0, buf);
  BYTE fat[512 * 9];
  WORD sector;
  WORD clusterToFetch = 0;
  char currentDirectory[256];
  strcpy(currentDirectory, "/", 2);

  if (dir != 0) {
    Debug("Reading sector\n");
    FloppyReadSector(dir, buf);
    FATDirectoryEntry* e = FATReadDirectory(buf);
    count = 0;
    while (e != NULL) {
      Debug("Current directory is '%s'\n", currentDirectory);
      Debug("Current file is '%s' %d\n", e->name, e->attributes);
      Debug("Looking for '%s'\n", name);
      // If this is the correct directory, start filling in entries
      if (!strcmp(currentDirectory, name)) {
        Debug("Found %s, count is %d\n", e->name, count);
        strcpy(dirimpl->dirents[count].d_name, e->name, strlen(e->name));

        // Fill in some extra fields of the dirents that are needed by stat
        dirimpl->dirents[count].st_size = e->size;
        dirimpl->dirents[count].st_mode = e->attributes & FAT_ATTR_DIRECTORY
                                              ? S_IFDIR
                                              : S_IFREG; // TODO other types
        e = e->next;
        ++count;
      } else {
        Debug("Not a match\n");
        if (!strcmp(name, e->name)) {
          Debug("Found matching dir '%s', clusters are %d %d \n", e->name,
                e->firstClusterLow, e->firstClusterHigh);
          // TODO support multiple levels deep
          strcpy(currentDirectory, e->name, strlen(e->name));
          Debug("New currentDir = '%s'\n", currentDirectory);

          for (int i = 1; i < 10; ++i) {
            Debug("Reading sector %d to %u\n", i, fat + (i - 1) * 512);
            FloppyReadSector(i, fat + (i - 1) * 512);
          }

          BYTE* foo = KMalloc(1024);
          FATReadFile(foo, &s, fat, e->firstClusterLow);
          FATDirectoryEntry* e = FATReadDirectory(foo);
          // while(e != NULL) {
          //   Debug("Found in dir: %s\n", e->name);
          //   e = e->next;
          // }
          //   // 451 0
          //   BYTE* foo = KMalloc(48); // e->size nto used for directories
          //   WORD absoluteCluster = e->firstClusterLow - 2 + 33;
          //   // FATReadFile(foo, &s, fat, e->firstClusterLow);

          // // BYTE buf2[512];
          //   WORD sector = FATSectorForCluster(&s, absoluteCluster);
          //   Debug("found sector %d\n", sector);
          //   FloppyReadSector(sector, buf);
          //   Debug("Done reading sector\n");
          //   // Debug(foo); // foo contains file contents
          //   Debug("***");
          //   FATDirectoryEntry* e = FATReadDirectory(buf);
          //   while(e != NULL) {
          //     Debug("Found in dir: %s\n", e->name);
          //     e = e->next;
          //   }

        } else {
          e = e->next;
        }
      }
    }
  }
  dirimpl->Count = count;
  dirimpl->Current = 0;
  return 0;
}

char* FloppyReadFile(char* name, int* size) // todo get siz
{
  Debug("Start of read\n");
  int i;
  BYTE buf[512];
  FloppyReadSector(0, buf);
  BYTE fat[512 * 9];
  /*for(i = 0; i < 70; ++i)
      KPrint("%c", buf[i]);*/
  WORD sector;

  WORD clusterToFetch = 0;

  if (dir != 0) {
    Debug("Reading sector\n");
    FloppyReadSector(dir, buf);
    FATDirectoryEntry* e = FATReadDirectory(buf);
    while (e != NULL) {
      // KPrint("  %s%s", e->name, e->attributes & FAT_ATTR_DIRECTORY ? "/" :
      // ""); KPrint("     "); if(e->attributes & FAT_ATTR_READ_ONLY)
      //     KPrint("Read-only ");
      // if(e->attributes & FAT_ATTR_HIDDEN)
      //     KPrint("Hidden ");
      // if(e->attributes & FAT_ATTR_SYSTEM)
      //     KPrint("System ");
      // if(e->attributes & FAT_ATTR_VOLUME_ID)
      //     KPrint("Volume id ");
      // // // KPrint(" %d sector ", FATSectorForCluster(&s,
      // e->firstClusterLow)); KPrint(" %d cluster ", e->firstClusterLow);
      // KPrint("  %d B", e->size);

      // KPrint("\n");

      if (!strcmp((const char*)e->name, name)) {
        Debug("Found cluster %d %s\n", e->firstClusterLow, name);
        clusterToFetch = e->firstClusterLow;
        *size = e->size;
        break;
      }

      e = e->next;
    }

    // Could not find file
    if (clusterToFetch == 0) {
      Debug("Could not find file '%s'\n", name);
      return NULL;
    }

    for (i = 1; i < 10; ++i) {
      Debug("Reading sector %d to %u\n", i, fat + (i - 1) * 512);
      FloppyReadSector(i, (char*)(fat + (i - 1) * 512));
    }
    Debug("Allocating %u\n", *size);
    char* fileData = KMalloc(*size);

    FATReadFile((BYTE*)fileData, &s, fat, clusterToFetch);
    return fileData;
  }

  return NULL;
}

// {
//   return FATFirstDataSector(bs) + (cluster - 2) * bs->sectorsPerCluster;
// }

STATUS FATReadFile(BYTE* buffer, FAT12BootSector* bs, BYTE* fat, WORD cluster) {
  if (buffer == NULL) {
    return S_FAIL;
  }

  WORD clustersRead = 0;

  while (1) {
    WORD sector = FATSectorForCluster(bs, cluster);
    FloppyReadSector(sector, (char*)(buffer + (clustersRead * 512)));

    WORD nextCluster = FATGetNextCluster(fat, cluster);
    Debug("Current: %u Next: %u  Count: %u\n", cluster, nextCluster,
          clustersRead);
    if (nextCluster >= 0xFF8 || nextCluster == 0xFF0) {
      Debug("No more clusters\n");
      break;
    }
    cluster = nextCluster;
    clustersRead++;
  }

  return S_OK;
}

WORD FATGetNextCluster(BYTE* fat, WORD cluster) {
  Debug("Cluster: %u\n", cluster);
  WORD offset = cluster + cluster / 2;
  Debug("Offset: %u\n", offset);
  BYTE first = *(BYTE*)&fat[offset];
  BYTE second = *(BYTE*)&fat[offset + 1];
  Debug("%d %d\n", first, second);
  WORD fatValue = second * 256 + first;

  if (cluster & 0x0001) {
    fatValue = fatValue >> 4;
  } else {
    fatValue = fatValue & 0x0FFF;
  }
  Debug("Next cluster: %u\n", fatValue);
  return fatValue;
}

/* Initialize floppy driver */
STATUS FloppyInit(void) {
  BYTE floppyInfo, driveA, driveB;
  IoWritePortByte(0x70, 0x10);
  floppyInfo = IoReadPortByte(0x71);

  driveA = floppyInfo >> 4;
  driveB = floppyInfo & 0xF;

  drives[0].present = driveA != NO_FLOPPY;
  drives[1].present = driveB != NO_FLOPPY;

  KPrint("Floppy A: id %d, type %s\n", driveA, floppyTypeString[driveA]);
  KPrint("Floppy B: id %d, type %s\n", driveB, floppyTypeString[driveB]);

  InstallIrqHandler(FLOPPY_IRQ, FloppyHandler);

  int i;
  for (i = 0; i < 2; ++i) {
    if (drives[i].present) {
      DiskReset(&drives[i]);

      GetDriveStatus(&drives[i]);

      /*if(drives[i].msr & MRQ)
          KPrint("Ready - ");
      else
          KPrint("Not ready - ");

      if(drives[i].msr & DIO)
          KPrint("controller to cpu - ");
      else
          KPrint("cpu to controller - ");

      if(drives[i].msr & NDMA)
          KPrint("no DMA - ");
      else
          KPrint("DMA - ");

      if(drives[i].msr & BUSY)
          KPrint("busy");
      else
          KPrint("not busy");

      KPrint("\n");*/

      drives[i].device.Name = KMalloc(16);
      sprintf(16, drives[i].device.Name, "floppy%d", i);
      drives[i].device.Status = DEVICE_OPEN;
      drives[i].device.OpenDir = FloppyReadDirectory;
      //   drives[i].device.Open = FloppyOpen;
      drives[i].device.Read = FloppyReadFile;
      drives[i].device.Stat = FloppyStat;
      DeviceRegister(&drives[i].device);
    }
  }
  return S_OK;
}

/* Destroy floppy driver */
STATUS FloppyDestroy(void) {
  RemoveIrqHandler(FLOPPY_IRQ);
  return S_OK;
}
