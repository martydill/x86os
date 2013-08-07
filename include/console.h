
/*
* Console.h
* Include file for the console mode text driver
*/

#ifndef DRV_CONSOLE_H
#define DRV_CONSOLE_H

#include <kernel.h>

#define CONSOLE_WIDTH	80
#define CONSOLE_HEIGHT	25
#define CONSOLE_BASE		0xb8000
#define CONSOLE_SIZE 		(CONSOLE_WIDTH * CONSOLE_HEIGHT * 2)
#define NUM_CONSOLES	8


typedef struct
{
    unsigned char* buffer;		/* Location of buffer in video memory */
    WORD cursorX, cursorY; 	/* Coordinates of cursor */
    BYTE color;
} Console;



STATUS ConInit(void);
STATUS ConActivateConsole(unsigned int number);
STATUS ConMoveCursor(WORD newX, WORD newY);
STATUS ConClearScreen(void);
STATUS ConDisplayString(const char* str, WORD x, WORD y);
STATUS ConDestroy(void);
STATUS ConBackspace();
STATUS ConGetCursorPosition(POINT* point);

#endif
