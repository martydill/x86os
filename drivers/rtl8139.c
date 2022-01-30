
#include <kernel_shared.h>
#include <kernel.h>
#include <floppy.h>
#include <io.h>
#include <interrupt.h>
#include <device.h>
#include <fat.h>
#include <dma.h>
#include <elf.h>
#include "pci.h"

#define RTL_CONFIG1_OFFSET 0x52

Device device;

STATUS Rtl8139Init(PciDevice* pciDevice) {
  KPrint("Starting RTL8139 init\n");
  const DWORD baseAddress = pciDevice->bar0;
  IoWritePortByte(baseAddress + RTL_CONFIG1_OFFSET, 0);

  KPrint("Done RTL8139 init\n");
  return S_OK;
}

STATUS Rtl8139Destroy(void) { return S_OK; }
