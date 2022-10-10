
#include <kernel.h>
#include <interrupt.h>
#include <mm.h>
#include <process.h>

/* Kernel is loaded at 0x101000, we need to be above that ... */
#define MEM_FILL_CHAR 'X'

unsigned int counter = 0;
unsigned int BaseMallocAddress;

// 4MB page -> Process ID map
WORD PhysicalMemoryToProcessMap[NUM_PAGE_DIRECTORY_ENTRIES];
PageDirectory* CurrentPageDirectory = NULL;
PageDirectory* kernelPageDirectory;

extern char _kernelEndAddress[];

void* KMallocWithTag(unsigned int numBytes, char* tag) {
  void* pointer = NULL;
  Assert(tag != NULL);

  pointer = (void*)counter;
  Debug("Allocated %u bytes at %u\n", numBytes, pointer);
  Memset(pointer, MEM_FILL_CHAR, numBytes);
  counter += numBytes;

  return pointer;
}

void* KMallocWithTagAligned(unsigned int numBytes, char* tag, int alignTo) {
  void* pointer = NULL;
  Assert(tag != NULL);

  pointer = (void*)counter;

  int diff = alignTo - (DWORD)pointer % alignTo;
  pointer += diff;

  Memset(pointer, MEM_FILL_CHAR, numBytes);
  counter += numBytes + diff;

  return pointer;
}

void* KMalloc(unsigned int numBytes) {
  return KMallocWithTag(numBytes, "BASE");
}

int MMGetNextFreePage() {
  for (int i = 16; i < NUM_PAGE_DIRECTORY_ENTRIES; ++i) {
    if (PhysicalMemoryToProcessMap[i] < 1) {
      Debug("MM: Next free page is %d %d\n", i, i * 4096 * 1024);
      return i;
    }
  }
}
// Returns the physical adddress of the next free page
DWORD MMGetNextFreePageAddress() {
  for (int i = 16; i < NUM_PAGE_DIRECTORY_ENTRIES; ++i) {
    if (PhysicalMemoryToProcessMap[i] < 1) {
      Debug("MM: Next free page is %d %d\n", i, i * 4096 * 1024);
      return i * 4096 * 1024;
    }
  }
  Assert(0);
}

void* KMallocInProcess(Process* p, unsigned int numBytes) {
  void* pointer = p->CurrentMemPtr;
  p->CurrentMemPtr += numBytes;
  return pointer;
}

// fixme - actually free
void KFree(void* pointer) {
  if (pointer == NULL)
    return;

  return;
}

STATUS MMAllocatePageToProcess(WORD page, WORD process) {
  Assert(page < NUM_PAGE_DIRECTORY_ENTRIES);
  Assert(process > 0);

  Debug("MM: Assigned page %u to process %u\n", page, process);
  PhysicalMemoryToProcessMap[page] = process;
}

int MMGetPageForProcess(WORD process) {
  Assert(process > 0);

  for (int i = 0; i < NUM_PAGE_DIRECTORY_ENTRIES; ++i) {
    if (PhysicalMemoryToProcessMap[i] == process) {
      return i;
    }
  }
  Assert(0)
}

STATUS MMFreePage(WORD page) {
  Assert(page > 16 && page < NUM_PAGE_DIRECTORY_ENTRIES);
  Debug("MM: freed page %u\n", page);
  PhysicalMemoryToProcessMap[page] = 0;
  Memset(4096 * 1024 * 1024 * page, MEM_FILL_CHAR, 4096 * 1024 * 1024);
}

STATUS MMMapPageToProcess(PageDirectory* pd, WORD page, WORD process) {
  if (page >= NUM_PAGE_DIRECTORY_ENTRIES) {
    return S_FAIL;
  }

  // Keep track of what process owns it
  // PhysicalMemoryToProcessMap[page] = process;
  // Debug("MM: Page %d now belongs to %d\n", page, process);
  // Debug("Marking page %d as present for page directory %u and process %u\n",
  // page, pd, process); Mark as present
  pd->Entries[page] |= 1 << 0;
  asm volatile("invlpg  (%0)" ::"r"(pd->Entries[page]) : "memory");
  return S_OK;
}

void MMMap(PageDirectory* pageDirectory, int virtualPage, int physicalPage,
           ProcessId processId) {
  int pageSize = 1024 * 1024 * 4;
  Debug("Mapping %d-%d to %d-%d for process %d\n", virtualPage * pageSize,
        (virtualPage + 1) * pageSize, physicalPage * pageSize,
        (physicalPage + 1) * pageSize, processId);

  pageDirectory->Entries[virtualPage] = (physicalPage * 1024 * 1024 * 4);
  pageDirectory->Entries[virtualPage] |= PDE_PRESENT;
  pageDirectory->Entries[virtualPage] |= PDE_WRITABLE;
  pageDirectory->Entries[virtualPage] |= PDE_SUPERVISOR;
  pageDirectory->Entries[virtualPage] |= PDE_4MB_PAGES;

  MMMapPageToProcess(pageDirectory, virtualPage, processId);

  kernelPageDirectory->Entries[physicalPage] = (physicalPage * 1024 * 1024 * 4);
  kernelPageDirectory->Entries[physicalPage] |= PDE_PRESENT;
  kernelPageDirectory->Entries[physicalPage] |= PDE_WRITABLE;
  kernelPageDirectory->Entries[physicalPage] |= PDE_SUPERVISOR;
  kernelPageDirectory->Entries[physicalPage] |= PDE_4MB_PAGES;
  MMMapPageToProcess(kernelPageDirectory, physicalPage, 0);
}

void MMInitializePageDirectory(PageDirectory* pageDirectory) {
  Debug("Initializing page directory %d %d\n", (unsigned int)pageDirectory,
        pageDirectory->Entries);
  for (int i = 0; i < NUM_PAGE_DIRECTORY_ENTRIES; ++i) {
    pageDirectory->Entries[i] = (i * 1024 * 1024 * 4);
    pageDirectory->Entries[i] |= PDE_WRITABLE;
    pageDirectory->Entries[i] |= PDE_SUPERVISOR;
    pageDirectory->Entries[i] |= PDE_4MB_PAGES;

    MMMapPageToProcess(pageDirectory, i, 0);

    // Map first 16 * 4MB pages to kernel space
    if (i < 16) {
      pageDirectory->Entries[i] |= PDE_PRESENT;
      MMMapPageToProcess(kernelPageDirectory, i, 0);
    }
  }
}

void MMInitializePageTables(PageDirectory* pageDirectory) {
  Debug("Initializing page tables for directory %d\n", pageDirectory);
  unsigned int* pageTable =
      (unsigned int*)((unsigned int*)pageDirectory + 0x1000);
  int address = 0;
  int i = 0;
  for (int pd = 0; pd < NUM_PAGE_DIRECTORY_ENTRIES; ++pd) {
    for (i = 0; i < NUM_PAGE_TABLE_ENTRIES; ++i) {
      pageTable[i] = ((1024 * 1024 * 4) * pd + (i * 0x1000)) | 3;
    }

    pageDirectory->Entries[pd] = (unsigned int)pageTable;
    pageDirectory->Entries[pd] |= 3;
    pageTable = pageTable + (4096);

    Debug("%d %d %d\n", pageTable, pd, i);
  }
}

void MMSetPageDirectory(PageDirectory* pageDirectory) {
  if (pageDirectory == CurrentPageDirectory) {
    return;
  }
  Debug("Switching to page directory at %u\n", pageDirectory);
  asm volatile("mov %0, %%cr3" ::"r"(pageDirectory));

  CurrentPageDirectory = pageDirectory;
  Debug("Done switch\n");
}

void MMEnablePaging(PageDirectory* pageDirectory) {
  // Enable pse for 4mb pages
  unsigned int cr4;
  asm volatile("mov %%cr4, %0" : "=b"(cr4));
  cr4 |= 0x00000010;
  asm volatile("mov %0, %%cr4" ::"b"(cr4));

  MMSetPageDirectory(pageDirectory);

  Debug("Enabling paging\n");
  // reads cr0, switches the "paging enable" bit, and writes it back.
  unsigned int cr0;
  asm volatile("mov %%cr0, %0" : "=b"(cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" ::"b"(cr0));

  Debug("Done\n");
}

void MMPageFaultHandler(Registers* registers) {
  const unsigned int code = registers->errorCode;
  unsigned int cr2;
  asm volatile("mov %%cr2, %0" : "=b"(cr2));

  Debug("Caught page fault for eip %u\r\n", registers->eip);
  if (code & (1 << 0))
    Debug("Protection violation\r\n");
  else
    Debug("Page is not present\r\n");

  if (code & (1 << 1))
    Debug("Page fault caused by write\r\n");
  else
    Debug("Page fault caused by read\r\n");

  if (code & (1 << 2))
    Debug("Ring 3\r\n");
  else
    Debug("Ring 0\r\n");

  Debug("Page fault address: %u\r\n", cr2);

  if ((code & (1 << 0)) == 0) {
    Debug("Not present, mapping\n");
    const unsigned int page = cr2 / (1024 * 1024 * 4);
    Debug("Page %u\n", page);

    if (code & (1 << 2)) {
      Process* p = ProcessGetActiveProcess();
      Debug("Mapping for user page directory %u\n", &p->PageDirectory);
      MMMapPageToProcess(p->PageDirectory, page, 1);
      // MMMapPageToProcess(kernelPageDirectory, page, 1);
      // asm volatile("mov %0, %%cr3":: "b"(&p->PageDirectory));
    } else {
      Debug("Mapping for kernel page directory %u\n", kernelPageDirectory);
      MMMapPageToProcess(kernelPageDirectory, page, 1);
    }
  }
}

void MMInstallPageFaultHandler() {
  InstallInterruptHandler(14, MMPageFaultHandler);
}

void MMInitializePaging() {
  int i;
  Debug("Initializing page directory\r\n");
  Debug("End: %d\r\n", (unsigned int)_kernelEndAddress);
  kernelPageDirectory =
      (PageDirectory*)((unsigned int)_kernelEndAddress & 0xFFFFF000) + 0x1000;

  Memset((BYTE*)PhysicalMemoryToProcessMap, 0,
         sizeof(PhysicalMemoryToProcessMap));

  MMInstallPageFaultHandler();
  MMInitializePageDirectory(kernelPageDirectory);
  for (int i = 0; i < 16; ++i) {
    MMMapPageToProcess(kernelPageDirectory, i, 0);
  }
  // MMInitializePageTables(kernelPageDirectory);
  MMEnablePaging(kernelPageDirectory);

  // Available memory starts at kernel end + size of page directory and page
  // tables
  BaseMallocAddress = (unsigned int)kernelPageDirectory + (4096 * 1024) + 1024;
  counter = BaseMallocAddress;
}
