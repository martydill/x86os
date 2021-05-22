
/*
 * pci.c
 * pci driver
 */

#include <kernel.h>
#include <pci.h>
#include <interrupt.h>
#include <io.h>
#include <console.h>
#include <device.h>

char PCI_VENDOR_NAMES[65535][32];

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
} PciDevice;

Device pciDevice;

DWORD PciReadConfigByte(BYTE bus, BYTE device, BYTE function, BYTE reg) {
  DWORD busAddress = bus << 16;
  DWORD deviceAddress = device << 11;
  DWORD functionAddress = function << 8;
  DWORD rawAddress =
      busAddress | deviceAddress | functionAddress | 0x80000000 | (reg << 2);
  IoWritePortDword(0xCF8, rawAddress);
  DWORD result = IoReadPortDword(0xCFC);
  return result;
}

// https://pci-ids.ucw.cz/read/PD/
char* PciGetClass(BYTE class) {
  switch (class) {
  case 0x00:
    return "Unclassified device	";
  case 0x02:
    return "Network controller";
  case 0x03:
    return "Display controller";
  case 0x06:
    return "Bridge";
  case 0x08:
    return "Generic system peripheral";
  default:
    return "Unknown";
  }
}

// http://pciids.sourceforge.net/v2.2/pci.ids
char* PciGetVendor(WORD vendorId) {
  switch (vendorId) {
  case 0x8086:
    return "Intel Corporation";
  case 0x80EE:
    return "InnoTek Systemberatung GmbH";
  case 0x1022:
    return "Advanced Micro Devices, Inc. ";
  default:
    return "Unknown";
  }
}

char* PciGetDevice(WORD vendorId, WORD deviceId) {
  switch (vendorId) {
  case 0x8086:
    switch (deviceId) {
    case 0x100E:
      return "82540EM Gigabit Ethernet Controller";
    case 0x1237:
      return "440FX - 82441FX PMC ";
    case 0x7000:
      return "82371SB PIIX3 ISA";
    case 0x7113:
      return "82371AB/EB/MB PIIX4 ACPI";
    default:
      return "Unknown";
    }

  case 0x80EE:
    switch (deviceId) {
    case 0xBEEF:
      return "VirtualBox Graphics Adapter";
    case 0xCAFE:
      return "VirtualBox Guest Service";
    default:
      return "Unknown";
    }

  case 0x1022:
    switch (deviceId) {
    case 0x2000:
      return "79c970 [PCnet32 LANCE]";
    default:
      return "Unknown";
    }

  default:
    return "Unknown";
  }
}

const WORD PCI_INVALID_VENDOR = 0xFFFF;

/* Initialize pci device */
STATUS PciInit(void) {
  pciDevice.Name = "PCI";
  pciDevice.Status = 0;
  pciDevice.Status |= DEVICE_OPEN;
  DeviceRegister(&pciDevice);

  for (BYTE bus = 0; bus < 0xFF; ++bus) {
    for (BYTE device = 0; device < 0x16; ++device) {
      DWORD rawBytes[4];
      for (int i = 0; i < 4; ++i) {
        DWORD result = PciReadConfigByte(bus, device, 0, i);
        rawBytes[i] = result;
      }
      PciDevice dev;
      Memcopy((BYTE*)&dev, (BYTE*)&rawBytes, sizeof(PciDevice));
      if (dev.vendorId == PCI_INVALID_VENDOR)
        continue;

      KPrint("%s %d, %s\n", PciGetVendor(dev.vendorId), dev.deviceId,
             PciGetDevice(dev.vendorId, dev.deviceId),
             PciGetClass(dev.classCode));
    }
  }
  return S_OK;
}

/* Destroy keyboard device */
STATUS PciDestroy(void) {
  DeviceUnregister(&pciDevice);
  return S_OK;
}
