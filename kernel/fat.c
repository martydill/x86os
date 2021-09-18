
/*
 * http://www.microsoft.com/whdc/system/platform/firmware/fatgen.mspx
 */

#include <kernel.h>
#include <fat.h>

#define READ_BYTE(BUFFER, X)                                                   \
  { BUFFER = *X++; }
#define READ_WORD(BUFFER, X)                                                   \
  {                                                                            \
    BUFFER = FATReadWord(X);                                                   \
    X += 2;                                                                    \
  }

#define READ_DWORD(BUFFER, X)                                                  \
  {                                                                            \
    unsigned char buf[4];                                                      \
    buf[0] = *X++;                                                             \
    buf[1] = *X++;                                                             \
    buf[2] = *X++;                                                             \
    buf[3] = *X++;                                                             \
    Memcopy(&BUFFER, buf, 4);                                                  \
  }

WORD FATReadWord(char* buffer) {
  unsigned int low = (unsigned int)(unsigned char)(*buffer);
  unsigned int high = (unsigned int)(unsigned char)(*(buffer + 1));
  unsigned int result = (high << 8) + low;

  return (WORD)result;
}

FAT12BootSector s;
int FATFirstDataSector(FAT12BootSector* bs) {
  DWORD sizeOfRootDir =
      (bs->numRootDirectoryEntries * 32 + (bs->bytesPerSector - 1)) /
      bs->bytesPerSector;
  DWORD firstSector = sizeOfRootDir + bs->numReservedSectors +
                      bs->sectorsPerFAT * bs->numFATCopies;
  return firstSector;
}

int FATSectorForCluster(FAT12BootSector* bs, int cluster) {
  return FATFirstDataSector(bs) + (cluster - 2) * bs->sectorsPerCluster;
}

int FATParseBootSector(char* bootSector) {
  bootSector += 3; /* skip jump thing */

  Memcopy(&s.oemName, bootSector, 8);
  bootSector += 8;

  READ_WORD(s.bytesPerSector, bootSector);
  READ_BYTE(s.sectorsPerCluster, bootSector);
  READ_WORD(s.numReservedSectors, bootSector);
  READ_BYTE(s.numFATCopies, bootSector);
  READ_WORD(s.numRootDirectoryEntries, bootSector);
  READ_WORD(s.totalNumberOfSectors, bootSector);
  READ_BYTE(s.mediaDescriptorType, bootSector);
  READ_WORD(s.sectorsPerFAT, bootSector);
  READ_WORD(s.sectorsPerTrack, bootSector);
  READ_WORD(s.numHeads, bootSector);

  Debug("OEMName: %s\r\n", s.oemName);
  Debug("Bytes per sector: %d\r\n", (int)s.bytesPerSector);
  Debug("Sectors per cluster: %d\r\n", (int)s.sectorsPerCluster);
  Debug("Reserved sectors: %d\r\n", (int)s.numReservedSectors);
  Debug("FAT copies: %d\r\n", (int)s.numFATCopies);
  Debug("Number of Root directory entries: %d\r\n",
        (int)s.numRootDirectoryEntries);
  Debug("Total number of sectors: %d\r\n", (int)s.totalNumberOfSectors);
  Debug("Media Descriptor Type: %d\r\n", (int)s.mediaDescriptorType);
  Debug("Sectors per FAT: %d\r\n", (int)s.sectorsPerFAT);
  Debug("Sectors per track: %d\r\n", (int)s.sectorsPerTrack);
  Debug("Number of heads: %d\r\n", (int)s.numHeads);

  Debug("First data sector: %d\n", FATFirstDataSector(&s));
  return s.numReservedSectors + (s.numFATCopies * s.sectorsPerFAT);
}

STATUS GetShortName(unsigned char* dest, unsigned char* name) {
  Assert(dest != NULL && name != NULL);
  char fileName[9];
  char extension[4];

  int i;
  for (i = 0; i < 8; ++i) {
    if (name[i] == ' ')
      break;

    fileName[i] = tolower(name[i]);
  }
  fileName[i] = '\0';

  for (i = 8; i < 11; ++i) {
    if (name[i] == ' ')
      break;

    extension[i - 8] = tolower(name[i]);
  }

  extension[i - 8] = '\0';
  if (extension[0] == '\0')
    sprintf(12, dest, "%s", fileName);
  else
    sprintf(12, dest, "%s.%s", fileName, extension);

  return S_OK;
}

FATDirectoryEntry* FATReadDirectory(char* directorySector) {
  FATDirectoryEntry* head = NULL;
  FATDirectoryEntry* e = NULL;
  FATDirectoryEntry* current = NULL;
  do {
    if (*directorySector == 0) {
      break;
    }

    if (*directorySector == FREE_DIRECTORY_ENTRY) {
      directorySector += 32;
      continue;
    }

    e = KMalloc(sizeof(FATDirectoryEntry));
    if (head == NULL) {
      head = e;
    } else if (current) {
      current->next = e;
    }
    current = e;
    GetShortName(&e->name, directorySector);
    directorySector += 11;
    READ_BYTE(e->attributes, directorySector);
    READ_BYTE(e->reserved, directorySector);
    READ_BYTE(e->crtTime, directorySector);
    READ_WORD(e->creationTime, directorySector);
    READ_WORD(e->creationDate, directorySector);
    READ_WORD(e->lastAccessDate, directorySector);
    READ_WORD(e->firstClusterHigh, directorySector);
    READ_WORD(e->writeTime, directorySector);
    READ_WORD(e->writeDate, directorySector);
    READ_WORD(e->firstClusterLow, directorySector);
    READ_DWORD(e->size, directorySector);
    e->next = NULL;
  } while (1);

  return head;
}
