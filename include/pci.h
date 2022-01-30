
#ifndef PCI_H
#define PCI_H

typedef struct PciDevice_S {
  WORD vendorId;
  WORD deviceId;

  WORD command;
  WORD status;

  BYTE revisionId;
  BYTE progInterface;
  BYTE subclass;
  BYTE classCode;

  BYTE cacheLineSize;
  BYTE latencyTimer;
  BYTE headerType;
  BYTE bist;
  DWORD bar0;
  DWORD bar1;
  DWORD bar2;
  DWORD bar3;
  DWORD bar4;
  DWORD bar5;
} PciDevice;

DWORD PciReadConfigByte(BYTE bus, BYTE device, BYTE function, BYTE reg);
STATUS PciInit(void);
STATUS PciDestroy(void);

#endif
