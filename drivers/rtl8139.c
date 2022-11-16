#define DEBUG 1

/*
rtl8139.c
A simple driver for the rtl8139 network card.
References:
https://people.freebsd.org/~wpaul/RealTek/spec-8139c%28160%29.pdf
https://www.cs.usfca.edu/~cruse/cs326f04/RTL8139_ProgrammersGuide.pdf
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

#define ETHERTYPE_ARP 2054

#define RBSTART 0x30

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

#define RTL_RCR_ACCEPT_BROADCAST (1 << 3)
#define RTL_RCR_ACCEPT_MULTICAST (1 << 2)
#define RTL_RCR_ACCEPT_PHYSICAL_MATCH (1 << 1)
#define RTL_RCR_ACCEPT_PHYSICAL_ADDRESS (1 << 0)

#define RTL_COMMAND_RST (1 << 4)
#define RTL_COMMAND_RE (1 << 3)
#define RTL_COMMAND_TE (1 << 2)

#define IMR_ROK 1 << 0
#define IMR_TOK 1 << 2

#define ISR_ROK 1 << 0
#define ISR_TOK 1 << 2

#define RTL_IMR 0x03c
#define RTL_ISR 0x03e

Device device;
PciDevice* rtl = 0;

typedef struct {
  DWORD TsadOffset;
  DWORD TsdOffset;
  char Buffer[1792];
} __attribute__((packed)) RtlTransmitBuffer;

typedef struct {
  char Buffer[8192 + 16];
  WORD ReceivePointerOffset;
} __attribute__((packed)) RTLReceiveBuffer;

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
RTLReceiveBuffer ReceiveBuffer;
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

  DWORD status = 0;
  status |= length & 0x1FFF;
  status |= 0 << 13;

  IoWritePortDword(baseAddress + transmitBuffer->TsdOffset, status);

  Rtl8139WaitForTransmit();

  CurrentTransmitBuffer++;
  if (CurrentTransmitBuffer >= 3) {
    CurrentTransmitBuffer = 0;
  }
  Debug("Switched to transmit buffer: %u\n", CurrentTransmitBuffer);

  return S_OK;
}
#define SWAP_WORD(x) (((x & 0x00ff) << 8) | ((x & 0xff00) >> 8))
#define SWAP_DWORD(x)                                                          \
  ((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) |           \
      ((x << 24) & 0xff000000)

void HandleEthernetPacket(EthernetPacket* packet) {
  WORD type = SWAP_WORD(packet->Length);
  Debug("Source: %s  Dest: %d:%d:%d:%d:%d:%d  Len: %d\n", packet->SrcMacAddress,
        packet->DestMacAddress[0], packet->DestMacAddress[1],
        packet->DestMacAddress[2], packet->DestMacAddress[3],
        packet->DestMacAddress[4], packet->DestMacAddress[5], type);

  if (type == ETHERTYPE_ARP) {
    ARPPacket* p = packet->Payload;
    Debug("Arp packet sha: %s spa: %d tha: %s tpa: %d\n", p->Sha, p->Spa,
          p->Tha, p->Tpa);
  }
}

void Rtl8139Interrupt(Registers* registers) {
  const DWORD baseAddress = rtl->bar0;
  // IoWritePortWord(baseAddress + RTL_IMR, 0);

  WORD result = IoReadPortWord(baseAddress + RTL_ISR);
  if (result == 0) {
    // No actual interrupt here, do nothing
    return;
  }
  if (result & ISR_TOK) {
    // Successful transmit
    Debug("Successful transmit\n");
    IoWritePortWord(baseAddress + RTL_ISR, ISR_TOK);
  }
  if (result & ISR_ROK) {
    Debug("Successful receive\n");
    // Receive buffer contains a header and a length, each two bytes. Skip
    // them.
    DWORD addr = ReceiveBuffer.Buffer + ReceiveBuffer.ReceivePointerOffset;
    EthernetPacket* ep = addr + 4;
    HandleEthernetPacket(ep);

    IoWritePortWord(baseAddress + RTL_ISR, ISR_ROK);
  }
  if (!(result & ISR_ROK) && !(result & ISR_TOK)) {
    Debug("Other: %d\n", result);
  }
}

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
  IoWritePortByte(baseAddress + RTL_COMMAND_OFFSET,
                  RTL_COMMAND_TE | RTL_COMMAND_RE);
  IoWritePortDword(baseAddress + RTL_TCR_OFFSET, 0x03000600);
  IoWritePortWord(baseAddress + 0x44, 0xf | (1 << 7));

  // Enable reads of everything for now
  IoWritePortDword(baseAddress + RTL_RCR_OFFSET,
                   //  RTL_RCR_ACCEPT_BROADCAST | RTL_RCR_ACCEPT_MULTICAST |
                   //  RTL_RCR_ACCEPT_PHYSICAL_ADDRESS |
                   RTL_RCR_ACCEPT_PHYSICAL_MATCH);

  // Enable read and write interrupts
  IoWritePortWord(baseAddress + RTL_IMR, IMR_ROK | IMR_TOK);

  // Configure read buffer
  ReceiveBuffer.ReceivePointerOffset = 0;
  IoWritePortDword(baseAddress + RBSTART, &ReceiveBuffer.Buffer);

  InstallIrqHandler(pciDevice->interruptLine, Rtl8139Interrupt);

  // BUild and send an arp packet
  ARPPacket arp;
  arp.HType = SWAP_WORD(1);
  arp.PType = SWAP_WORD(0x0800);
  arp.HLen = 6;
  arp.PLen = 4;
  arp.Oper = SWAP_WORD(1);
  Memcopy(arp.Sha, macAddress, 6);
  arp.Spa = 0; // SWAP_DWORD(3232235619);
  Memset(arp.Tha, 0xFF, sizeof(arp.Tha));
  arp.Tpa = SWAP_DWORD(3232235682);

  Rtl8139Send((char*)&arp);

  KPrint("Done RTL8139 init, interrupt %u %u %u, line %u, pin %u\n",
         pciDevice->maxLatency, pciDevice->minGrant, pciDevice->vendorId,
         pciDevice->interruptLine, pciDevice->interruptPin);
  return S_OK;
}

STATUS Rtl8139Destroy(void) { return S_OK; }
