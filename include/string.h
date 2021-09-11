
#ifndef STRING_H
#define STRING_H

#include <kernel.h>

/* String functions */
int strcmp(const char* string1, const char* string2);
int strlen(const char* string);
STATUS sprintf(int size, char* buffer, const char* format,
               ...); // todo standardize
STATUS Dosprintf(int size, char* buffer, const char* format, va_list args);

void Test_String(void);

#endif
