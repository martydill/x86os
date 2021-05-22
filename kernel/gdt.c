
/*
 * KeGdt.c
 * GDT functions
 */

#include <kernel.h>
#include <gdt.h>
#include <tss.h>

/* GDT data structures */
GDTEntry GDT[NUM_GDT_ENTRIES];
GDTPointer gdtPointer;

/* Builds a GDT entry */
STATUS SetGdtEntry(unsigned int number, DWORD baseAddress, DWORD limitAddress,
                   BYTE access, BYTE granularity) {
  Assert(number < NUM_GDT_ENTRIES);
  Assert(baseAddress <= limitAddress);

  GDT[number].BaseAddressLow = baseAddress & 0xFFFF;
  GDT[number].BaseAddressMiddle = (baseAddress >> 16) & 0xFF;
  GDT[number].BaseAddressHigh = (baseAddress >> 24) & 0xFF;

  GDT[number].LowerLimit = limitAddress & 0xFFFF;
  GDT[number].Granularity = (limitAddress >> 16) & 0x0F;

  GDT[number].Granularity = GDT[number].Granularity | (granularity & 0xF0);
  GDT[number].Access = access;

  return S_OK;
}

/* Sets up the GDT */
STATUS LoadGdt() {
  Assert(GDT != NULL);

  __asm__ __volatile__("lgdt %0" : "=m"(gdtPointer));

  __asm__ __volatile__("movw %ax, 0x10\n\t"
                       "movw %ds, %ax\n\t"
                       "movw %es, %ax\n\t"
                       "movw %fs, %ax\n\t"
                       "movw %gs, %ax\n\t"
                       "movw %ss, %ax\n\t"
                       "ljmp $0x08, $JumpHelper\n\t"
                       "JumpHelper:\n\t");

  return S_OK;
}
extern TSS GlobalTSS;

/* Initializes the GDT */
void InitializeGDT(void) {
  STATUS gdtResult;

  /* Set up magic GDT Pointer */
  gdtPointer.Limit = sizeof(struct GDTEntry_S) * NUM_GDT_ENTRIES - 1;
  gdtPointer.Base = (DWORD)&GDT;

  /* fixme: build numbers from granularity and access */

  /* Null GDT */
  gdtResult = SetGdtEntry(0, 0, 0, 0, 0);
  Assert(gdtResult == S_OK);

  /* Code segment */
  gdtResult = SetGdtEntry(1, LOW_ADDRESS, HIGH_ADDRESS, 0x9A, 0xCF);
  Assert(gdtResult == S_OK);

  /* Data segment */
  gdtResult = SetGdtEntry(2, LOW_ADDRESS, HIGH_ADDRESS, 0x92, 0xCF);
  Assert(gdtResult == S_OK);

  /* User-mode Code segment */
  gdtResult = SetGdtEntry(3, LOW_ADDRESS, HIGH_ADDRESS, 0xFA, 0xCF);
  Assert(gdtResult == S_OK);

  /* User-mode Data segment */
  gdtResult = SetGdtEntry(4, LOW_ADDRESS, HIGH_ADDRESS, 0xF2, 0xCF);
  Assert(gdtResult == S_OK);

  SetGdtEntry(5, &GlobalTSS, &GlobalTSS + sizeof(TSS) - 1, 0x89, 0x00);
  LoadTSS(&GDT[5]);
  LoadGdt();
  FlushTSS();
  return;
}
