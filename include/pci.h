
#ifndef PCI_H 
#define PCI_H


DWORD PciReadConfigByte(BYTE bus, BYTE device, BYTE function, BYTE reg);
STATUS PciInit(void);
STATUS PciDestroy(void);

#endif
