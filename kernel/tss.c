#include <kernel.h>
#include <gdt.h>
#include <tss.h>
TSS GlobalTSS;

STATUS LoadTSS(GDTEntry* gdt) {
  Assert(gdt != NULL);
  Memset((BYTE*)&GlobalTSS, 0, sizeof(TSS));
  GlobalTSS.SS0 = 0x10;
  GlobalTSS.ESP0 = 1024 * 1024 * 10; // KMalloc(1024 * 8); // 8k Kernel stack
  GlobalTSS.IOPB = sizeof(TSS) - 1;
  GlobalTSS.CS = 0x0b;
  GlobalTSS.SS = GlobalTSS.DS = GlobalTSS.ES = GlobalTSS.FS = GlobalTSS.GS =
      0x13;
  return S_OK;
}
