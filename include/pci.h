
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
  DWORD cardbus;
  WORD subsystemVendorId;
  WORD subsystemId;
  DWORD expansionRomBaseAddress;
  BYTE capabilitiesPointer;
  BYTE reserved[3];
  DWORD reserved2;
  BYTE interruptLine;
  BYTE interruptPin;
  BYTE minGrant;
  BYTE maxLatency;

  // Not actually part of the PCI spec, just extra accounting fields
  DWORD bus;
  DWORD device;
} __attribute__((packed)) PciDevice;

DWORD PciReadConfigByte(BYTE bus, BYTE device, BYTE function, BYTE reg);
STATUS PciInit(void);
STATUS PciDestroy(void);
STATUS PciEnableBusMaster(PciDevice* device);

#endif
