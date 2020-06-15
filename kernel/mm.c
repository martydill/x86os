
#include <kernel.h>
#include <interrupt.h>
#include <mm.h>

/* Kernel is loaded at 0x101000, we need to be above that ... */
#define MEM_FILL_CHAR	'X'


unsigned int counter = 0;
int pos = 8;
unsigned int BaseMallocAddress;// = kernelPageDirectory
// fixme - use actual dynamic memory allocationG114

// 4MB page -> Process ID map
WORD PhysicalMemoryToProcessMap[NUM_PAGE_DIRECTORY_ENTRIES];

void* KMallocWithTag(unsigned int numBytes, char* tag)
{
    void* pointer = NULL;
    Assert(tag != NULL);

    if(counter == 0)
    {
        Debug("Setting counter to %d\n", BaseMallocAddress);
        counter = BaseMallocAddress;
    }

    pointer = (void*)counter;

    Memset(pointer, MEM_FILL_CHAR, numBytes);
    counter += numBytes;

    return pointer;
}


void* KMalloc(unsigned int numBytes)
{
    return KMallocWithTag(numBytes, "BASE");
}


// fixme - actually free
void KFree(void* pointer)
{
    if(pointer == NULL)
        return;

    return;
}

//unsigned int page_aligned_end = (((unsigned int*)BASE_MALLOC_ADDRESS) & 0xFFFFF000) + 0x1000;

extern char _kernelEndAddress[];

extern char _physical_load_addr[];

PageDirectory* kernelPageDirectory;

STATUS MMMapPageToProcess(WORD page, WORD process) {

  if(page >= NUM_PAGE_DIRECTORY_ENTRIES) {
    return S_FAIL;
  }

  // Keep track of what process owns it
  PhysicalMemoryToProcessMap[page] = process;

  Debug("Marking page %d as present\n", page);
  // Mark as present
  kernelPageDirectory->Entries[page] |= 1 << 0; 
  // asm volatile("invlpg  (%0)" ::"r"(kernelPageDirectory->Entries): "memory");
  return S_OK;
}

void MMInitializePageDirectory(PageDirectory* pageDirectory)
{
    Debug("Initializing page directory %d %d\n", (unsigned int)pageDirectory, pageDirectory->Entries);
    //set each entry to not present
    unsigned int i = 0;
    for(i = 0; i < NUM_PAGE_DIRECTORY_ENTRIES; ++i)
    {
        pageDirectory->Entries[i] = (i * 1024 * 1024 * 4);// | (1 << 3) | (1 << 7);
        //attribute: supervisor level, read/write, not present.
        // pageDirectory->Entries[i] = 0;
        // pageDirectory->Entries[i] |= 1;
        // pageDirectory->Entries[i] |= 1 << 0;
        pageDirectory->Entries[i] |= 1 << 1;
        pageDirectory->Entries[i] |= 1 << 7;
        // Debug("%u %u\n", pageDirectory->Entries[i], i);
    }
    Memset(PhysicalMemoryToProcessMap, 0, sizeof(PhysicalMemoryToProcessMap));
    for(i = 0; i < 8; ++i) {
      MMMapPageToProcess(i, 1);
    }
}

void MMInitializePageTables(PageDirectory* pageDirectory)
{
    Debug("Initializing page tables for directory %d\n", pageDirectory);
    unsigned int* pageTable = (unsigned int*)((unsigned int*)pageDirectory + 0x1000);
    int address = 0;
    int i = 0;
    for(int pd = 0; pd < NUM_PAGE_DIRECTORY_ENTRIES; ++pd) {
      for(i = 0; i < NUM_PAGE_TABLE_ENTRIES; ++i)
      {
          pageTable[i] = ((1024 * 1024 * 4) * pd + (i * 0x1000)) | 3;
          // Debug("PT %d %d %d\n", i, pd, pageTable[i]);
          // address += NUM_PAGE_TABLE_ENTRIES * sizeof(int);
      }

      pageDirectory->Entries[pd] = pageTable;
      pageDirectory->Entries[pd] |= 3;
      pageTable = pageTable + (4096);

      Debug("%d %d %d\n", pageTable, pd, i);
    }
}

void MMEnablePaging(PageDirectory* pageDirectory)
{
    // Debug("Enabling large pages\n");

  // Enable pse for 4mb pages
    

      // asm volatile("movl %%cr4, %%eax");
      // asm volatile("or eax, 0x00000010");
      // asm volatile("movl cr4, eax");
     //mov eax, cr4
    // or eax, 0x00000010
    //mov cr4, eax

    unsigned int cr4;
    asm volatile("mov %%cr4, %0": "=b"(cr4));
    cr4 |= 0x00000010;
    asm volatile("mov %0, %%cr4":: "b"(cr4));

    Debug("Enabling paging\n");
    asm volatile("mov %0, %%cr3":: "b"(pageDirectory));
    //reads cr0, switches the "paging enable" bit, and writes it back.
    unsigned int cr0;
    asm volatile("mov %%cr0, %0": "=b"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "b"(cr0));

    Debug("Done\n");

}

void MMPageFaultHandler(Registers* registers)
{
    const unsigned int code = registers->errorCode;
    unsigned int cr2;
    asm volatile("mov %%cr2, %0": "=b"(cr2));

    Debug("Caught page fault\r\n");
    if(code & (1 << 0))
        Debug("Protection violation\r\n");
    else
        Debug("Page is not present\r\n");

    if(code & (1 << 1))
        Debug("Page fault caused by write\r\n");
    else
        Debug("Page fault caused by read\r\n");

    if(code & (1 << 2))
        Debug("Ring 3\r\n");
    else
        Debug("Ring 0\r\n");

    Debug("Page fault address: %u\r\n", cr2);

    if ((code & (1 << 0)) == 0) {
      Debug("Not present, mapping\n");
      const unsigned int page = cr2 / (1024 * 1024 * 4);
      Debug("Page %u\n", page);
      MMMapPageToProcess(page, 1);
    }
}

void MMInstallPageFaultHandler()
{
    InstallInterruptHandler(14, MMPageFaultHandler);
}

void MMInitializePaging()
{
    int i;
    Debug("Addr: %u\n", _physical_load_addr);
    Debug("Initializing page directory\r\n");
    Debug("End: %d\r\n", (unsigned int)_kernelEndAddress);
    kernelPageDirectory = (PageDirectory*)((unsigned int)_kernelEndAddress & 0xFFFFF000) + 0x1000;

    MMInstallPageFaultHandler();
    MMInitializePageDirectory(kernelPageDirectory);
    // MMInitializePageTables(kernelPageDirectory);
    MMEnablePaging(kernelPageDirectory);
    
    // Available memory starts at kernel end + size of page directory and page tables
    BaseMallocAddress = kernelPageDirectory + (4096 * 1024) + 1024;
    // Debug("%u %u\n", BaseMallocAddress, kernelPageDirectory);
    // for (i = 0; i < 1024; ++i) {
    //   const unsigned int z = KMalloc(1024 * i);
    //   Debug("Allocated %u bytes at %u", 1024 * i, z);
    // }
    // while(1) {}
    // int* x= (int*)989999999;
    // *x = 5;
    // i = *((unsigned int*)pageDirectory + 0x5000);
}
