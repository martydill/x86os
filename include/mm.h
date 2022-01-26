
#ifndef MM_H
#define MM_H

#define NUM_PAGE_DIRECTORY_ENTRIES 1024
#define NUM_PAGE_TABLE_ENTRIES 1024

#define PDE_PRESENT (1 << 0)
#define PDE_WRITABLE (1 << 1)
#define PDE_SUPERVISOR (1 << 2)
#define PDE_4MB_PAGES (1 << 7)

void* KMallocWithTag(unsigned int numBytes, char* tag);
void* KMalloc(unsigned int numBytes);
void KFree(void* pointer);
typedef struct Process_S Process;
void* KMallocInProcess(Process* p, unsigned int numBytes);

typedef struct {
  unsigned int Entries[NUM_PAGE_DIRECTORY_ENTRIES]
      __attribute__((aligned(4096)));
} PageDirectory;

#endif
