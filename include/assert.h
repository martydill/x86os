
#ifndef ASSERT_H
#define ASSERT_H


#include <kernel.h>

/* Assert function */
void DoAssert(const char* expression, const char* fileName, int lineNumber);

/* Assert macro */
#ifdef DEBUG
#define Assert(expression) if(!(expression)) { DoAssert(#expression, __FILE__, __LINE__); }
#else
#define Assert(expression) ;
#endif


#endif
