
#ifndef FAT_H
#define FAT_H

#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN 	0x02
#define FAT_ATTR_SYSTEM 	0x04
#define FAT_ATTR_VOLUME_ID 	0x08
#define FAT_ATTR_DIRECTORY	0x10
#define FAT_ATTR_ARCHIVE  	0x20
#define FAT_ATTR_LONG_NAME 	FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID

#define FREE_DIRECTORY_ENTRY	0xe5


typedef struct FAT12BootSector_S
{
    BYTE jumpToBootstrap[3];
    BYTE oemName[8];
    WORD bytesPerSector;
    BYTE sectorsPerCluster;
    WORD numReservedSectors;
    BYTE numFATCopies;
    WORD numRootDirectoryEntries;
    WORD totalNumberOfSectors;
    BYTE mediaDescriptorType;
    WORD sectorsPerFAT;
    WORD sectorsPerTrack;
    WORD numHeads;
    WORD numHiddenSectors;
    BYTE bootstrap[479];
    WORD Signature;
} FAT12BootSector;



typedef struct FATDirectoryEntry_S
{
    unsigned char name[12];
    BYTE attributes;
    BYTE reserved;
    BYTE crtTime;
    WORD creationTime;
    WORD creationDate;
    WORD lastAccessDate;
    WORD firstClusterHigh;
    WORD writeTime;
    WORD writeDate;
    WORD firstClusterLow;
    DWORD size;
    struct FATDirectoryEntry_S* next;
} FATDirectoryEntry;


int FATParseBootSector(char*);
#endif
