
#ifndef STRING_H
#define STRING_H

#include <kernel.h>

/* String functions */
int Strcmp(const char* string1, const char* string2);
int Strlen(const char* string);
STATUS Sprintf(int size, char* buffer, const char* format, ...);
STATUS DoSprintf(int size, char* buffer, const char* format, va_list args);

void Test_String(void);

#endif
