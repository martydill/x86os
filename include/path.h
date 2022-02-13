
#ifndef PATH_H
#define PATH_H

#include <kernel.h>

char* PathSkipFirstComponent(const char* path);
char* PathGetFirstComponent(const char* path);

#endif
