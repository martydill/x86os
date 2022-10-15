#define DEBUG

/*
rtl8139.c
A simple driver for the rtl8139 network card.
References:
https://people.freebsd.org/~wpaul/RealTek/spec-8139c%28160%29.pdf
https://wiki.osdev.org/RTL8139
https://www.wfbsoftware.de/realtek-rtl8139-network-interface-card/
https://en.wikipedia.org/wiki/Address_Resolution_Protocol
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
#include "pci.h"

#define RTL_CONFIG1_OFFSET 0x52
#define RTL_COMMAND_OFFSET 0x37

#define RTL_TSD0_OFFSET 0x10
#define RTL_TSAD0_OFFSET 0x20

#define RTL_TSD1_OFFSET 0x14
#define RTL_TSAD1_OFFSET 0x24

#define RTL_TSD2_OFFSET 0x18
#define RTL_TSAD2_OFFSET 0x28

#define RTL_TSD3_OFFSET 0x1C
#define RTL_TSAD3_OFFSET 0x2C

#define RTL_TCR_OFFSET 0x40
#define RTL_RCR_OFFSET 0x44

#define RTL_COMMAND_RST (1 << 4)
#define RTL_COMMAND_RE (1 << 3)
#define RTL_COMMAND_TE (1 << 2)

Device device;
PciDevice* rtl;

typedef struct {
  DWORD TsadOffset;
  DWORD TsdOffset;
  char Buffer[1792];
} __attribute__((packed)) RtlTransmitBuffer;

// https://en.wikipedia.org/wiki/Ethernet_frame
typedef struct {
  BYTE DestMacAddress[6];
  BYTE SrcMacAddress[6];
  WORD Length;
  BYTE Payload[255];
} __attribute__((packed)) EthernetPacket;

typedef struct {
  WORD HType;
  WORD PType;
  BYTE HLen;
  BYTE PLen;
  WORD Oper;
  WORD Sha[3];
  DWORD Spa;
  WORD Tha[3];
  DWORD Tpa;
} __attribute__((packed)) ARPPacket;

RtlTransmitBuffer TransmitBuffers[4];
BYTE macAddress[6];
char tsad[255];
int CurrentTransmitBuffer = 0;

// Wait for the current transmission to complete
STATUS Rtl8139WaitForTransmit() {
  Assert(rtl);
  Assert(rtl->bar0);

  const DWORD baseAddress = rtl->bar0;
  const RtlTransmitBuffer* transmitBuffer =
      &TransmitBuffers[CurrentTransmitBuffer];
  while ((IoReadPortByte(baseAddress + transmitBuffer->TsdOffset) &
          (1 << 15)) != 0) {
  }

  return S_OK;
}

// Send the given chunk of data to the ether
STATUS Rtl8139Send(char* data) {
  Assert(data);
  Assert(rtl);
  Assert(rtl->bar0);
  Debug("Current transmit buffer: %u\n", CurrentTransmitBuffer);

  const DWORD baseAddress = rtl->bar0;
  const RtlTransmitBuffer* transmitBuffer =
      &TransmitBuffers[CurrentTransmitBuffer];

  EthernetPacket packet;

  Memset(packet.DestMacAddress, 255, 6);
  Memcopy(packet.SrcMacAddress, macAddress, sizeof(macAddress));
  Memcopy(packet.Payload, data, sizeof(ARPPacket));
  packet.Length = 0x0608;

  const int length = 64;
  Memcopy(transmitBuffer->Buffer, &packet, length);

  IoWritePortDword(baseAddress + transmitBuffer->TsadOffset,
                   transmitBuffer->Buffer);

  Debug("Sending %d bytes at '%s'\n", length, transmitBuffer->Buffer);
  DWORD status = 0;
  status |= length & 0x1FFF;
  status |= 0 << 13;
  IoWritePortDword(baseAddress + transmitBuffer->TsdOffset, status);
  Rtl8139WaitForTransmit();

  CurrentTransmitBuffer++;
  if (CurrentTransmitBuffer >= 3) {
    CurrentTransmitBuffer = 0;
  }
  KPrint("Switched to transmit buffer: %u\n", CurrentTransmitBuffer);

  return S_OK;
}
#define SWAP_WORD(x) ((x >> 8) | (x << 8))

STATUS Rtl8139Init(PciDevice* pciDevice) {
  KPrint("Starting RTL8139 init for %u %u\n", pciDevice->bar0, pciDevice->bar1);
  rtl = pciDevice;
  const DWORD baseAddress = pciDevice->bar0;

  // Set up transmit buffers
  Memset(&TransmitBuffers, 0, sizeof(RtlTransmitBuffer) * 4);

  TransmitBuffers[0].TsadOffset = RTL_TSAD0_OFFSET;
  TransmitBuffers[0].TsdOffset = RTL_TSD0_OFFSET;

  TransmitBuffers[1].TsadOffset = RTL_TSAD1_OFFSET;
  TransmitBuffers[1].TsdOffset = RTL_TSD1_OFFSET;

  TransmitBuffers[2].TsadOffset = RTL_TSAD2_OFFSET;
  TransmitBuffers[2].TsdOffset = RTL_TSD2_OFFSET;

  TransmitBuffers[3].TsadOffset = RTL_TSAD3_OFFSET;
  TransmitBuffers[3].TsdOffset = RTL_TSD3_OFFSET;

  // Turn on
  IoWritePortByte(baseAddress + RTL_CONFIG1_OFFSET, 0);

  // Reset
  IoWritePortByte(baseAddress + RTL_COMMAND_OFFSET, RTL_COMMAND_RST);
  while ((IoReadPortByte(baseAddress + RTL_COMMAND_OFFSET) & RTL_COMMAND_RST) !=
         0) {
  }

  // Get MAC Addres
  // TODO need hex support in KPrint to display it
  for (int i = 0; i < 6; ++i) {
    macAddress[i] = IoReadPortByte(baseAddress + i);
    KPrint("*%d*", macAddress[i]);
  }

  // Enable reads and writes
  IoWritePortByte(baseAddress + RTL_COMMAND_OFFSET, RTL_COMMAND_TE);
  IoWritePortDword(baseAddress + RTL_TCR_OFFSET, 0x03000600);

  ARPPacket arp;
  arp.HType = SWAP_WORD(1);
  arp.PType = SWAP_WORD(0x0800);
  arp.HLen = 6;
  arp.PLen = 4;
  arp.Oper = SWAP_WORD(1);
  Memcopy(arp.Sha, macAddress, 6);
  arp.Spa = 8156233232;
  Memset(arp.Tha, 0, sizeof(arp.Tha));
  arp.Tpa = 134744072; // 3232235521;
  Rtl8139Send((char*)&arp);

  KPrint("Done RTL8139 init\n");
  return S_OK;
}

STATUS Rtl8139Destroy(void) { return S_OK; }
