
#ifndef MEMORY_H
#define MEMORY_H

#include <kernel.h>

/* Memory functions */
STATUS Memset(BYTE* string, BYTE value, int size);
STATUS Memcopy(BYTE* dest, const BYTE* src, int size);

void Test_Memory(void);

#endif
