
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
#define RTL_COMMAND_OFFSET 0x37
#define RTL_TSD0_OFFSET 0x10
#define RTL_TSAD0_OFFSET 0x20

#define RTL_TCR_OFFSET 0x40
#define RTL_RCR_OFFSET 0x44

#define RTL_COMMAND_RST (1 << 4)
#define RTL_COMMAND_RE (1 << 3)
#define RTL_COMMAND_TE (1 << 2)

Device device;
char tsad[255];

STATUS Rtl8139Init(PciDevice* pciDevice) {
  KPrint("Starting RTL8139 init for %u %u\n", pciDevice->bar0, pciDevice->bar1);
  const DWORD baseAddress = pciDevice->bar0;

  // Turn on
  IoWritePortByte(baseAddress + RTL_CONFIG1_OFFSET, 0);

  // Reset
  IoWritePortByte(baseAddress + RTL_COMMAND_OFFSET, RTL_COMMAND_RST);
  while ((IoReadPortByte(baseAddress + RTL_COMMAND_OFFSET) & RTL_COMMAND_RST) !=
         0) {
  }

  // Enable reads and writes
  IoWritePortByte(baseAddress + RTL_COMMAND_OFFSET, RTL_COMMAND_TE);
  IoWritePortDword(baseAddress + RTL_TCR_OFFSET, 0x03000600);

  IoWritePortDword(baseAddress + RTL_TSAD0_OFFSET, tsad);
  Memset(&tsad[0], '$', 100);

  DWORD status = 0;
  status |= 255 & 0x1FFF;
  status |= 0 << 13;
  IoWritePortDword(baseAddress + RTL_TSD0_OFFSET, status);
  KPrint("Done RTL8139 init\n");
  return S_OK;
}

STATUS Rtl8139Destroy(void) { return S_OK; }
