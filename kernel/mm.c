
#include <kernel.h>
#include <interrupt.h>

/* Kernel is loaded at 0x101000, we need to be above that ... */
#define BASE_MALLOC_ADDRESS		0X200000
#define MEM_FILL_CHAR	'X'


unsigned int counter = 0;
int pos = 8;

// fixme - use actual dynamic memory allocationG114

void* KMallocWithTag(unsigned int numBytes, char* tag)
{
    void* pointer = NULL;
    Assert(tag != NULL);

    if(counter == 0)
    {
        counter = BASE_MALLOC_ADDRESS;
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


#define NUM_PAGE_DIRECTORY_ENTRIES  1024
#define NUM_PAGE_TABLE_ENTRIES      1024


typedef struct
{
    unsigned int Entries[NUM_PAGE_DIRECTORY_ENTRIES];
} PageDirectory;


//unsigned int page_aligned_end = (((unsigned int*)BASE_MALLOC_ADDRESS) & 0xFFFFF000) + 0x1000;

extern char _kernelEndAddress[];

PageDirectory* pageDirectory;
unsigned int* firstPageTable;
void MMInitializePageDirectory()
{
    Debug("Initializing page directory\r\n");
    Debug("End: %d\r\n", (unsigned int)_kernelEndAddress);
    pageDirectory = (PageDirectory*)((unsigned int)_kernelEndAddress & 0xFFFFF000) + 0x1000;
    Debug("Page Directory Address: %d\r\n", (unsigned int)pageDirectory);
    //set each entry to not present
    int i = 0;
    for(i = 0; i < NUM_PAGE_DIRECTORY_ENTRIES; ++i)
    {
        //attribute: supervisor level, read/write, not present.
        pageDirectory->Entries[i] = 0 | 2;
    }
}

void MMInitializePageTables()
{
    Debug("Initializing page tables");
    firstPageTable = (unsigned int*)((unsigned int*)pageDirectory + 0x1000);
    int address = 0;
    int i = 0;
    for(i = 0; i < NUM_PAGE_TABLE_ENTRIES; ++i)
    {
        firstPageTable[i] = address | 3;
        address += NUM_PAGE_TABLE_ENTRIES * sizeof(int);
    }

    pageDirectory->Entries[0] = firstPageTable;
    pageDirectory->Entries[0] |= 3;
}

void MMEnablePaging()
{
    Debug("Enabling paging");
    asm volatile("mov %0, %%cr3":: "b"(pageDirectory));
    //reads cr0, switches the "paging enable" bit, and writes it back.
    unsigned int cr0;
    asm volatile("mov %%cr0, %0": "=b"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "b"(cr0));
}

void MMPageFaultHandler(Registers* registers)
{
    int code = registers->errorCode;
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

    unsigned int cr2;
    asm volatile("mov %%cr2, %0": "=b"(cr2));

    Debug("Page fault address: %u\r\n", cr2);
}

void MMInstallPageFaultHandler()
{
    InstallInterruptHandler(14, MMPageFaultHandler);
}

void MMInitializePaging()
{
    int i;
    MMInstallPageFaultHandler();
    MMInitializePageDirectory();
    MMInitializePageTables();
    MMEnablePaging();

    // i = *((unsigned int*)pageDirectory + 0x5000);
}
