
#ifndef MM_H
#define MM_H

#define NUM_PAGE_DIRECTORY_ENTRIES 1024
#define NUM_PAGE_TABLE_ENTRIES 1024

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
