
/*
 * RtlAssert.c
 * The run-time library's assert() function
 */

#include <kernel.h>
#include <console.h>

extern void KeHalt(void);

/* Called when an assertion fails. Prints expression, file name, and line number
 * to the screen, and halts. */
void DoAssert(const char* expression, const char* fileName, int lineNumber) {
  char buffer[256];
  ConClearScreen();

  sprintf(strlen(buffer), buffer,
          "Kernel assertion failed - %s, line %d: \"%s\"", fileName, lineNumber,
          expression);
  ConDisplayString(buffer, 0, 0);
  KeHalt();
}
