
#ifndef MM_H
#define MM_H

void* KMallocWithTag(unsigned int numBytes, char* tag);
void* KMalloc(unsigned int numBytes);
void KFree(void* pointer);

#endif
