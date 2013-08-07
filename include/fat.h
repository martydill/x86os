
#ifndef FAT_H
#define FAT_H


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


int FATParseBootSector(char*);
#endif
