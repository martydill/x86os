
/*
* Console.c
* Console text mode video driver
* References: http://my.execpc.com/~geezer/osd/cons/index.htm
*/

#include <kernel.h>
#include <io.h>
#include <console.h>
#include <device.h>

Console console[NUM_CONSOLES];
BYTE activeConsole;
Console* pActiveConsole;

Device consoleDevice;


STATUS ConsoleDeviceWrite(char* buffer, int numBytes)
{
    return S_OK;
}


/* Initialize console driver */
STATUS ConInit(void)
{
    int i;

    for(i = 0; i < NUM_CONSOLES; ++i)
    {
        console[i].buffer = (unsigned char*)(CONSOLE_BASE + (CONSOLE_SIZE * i));
        console[i].cursorX = 0;
        console[i].cursorY = 0;
        console[i].color = 0x0F;
        Memset(console[i].buffer, 0, CONSOLE_SIZE);
    }


    consoleDevice.Name = "Console";
    consoleDevice.Write = ConsoleDeviceWrite;
    consoleDevice.Status = 0;
    consoleDevice.Status |= DEVICE_CAN_WRITE;

    DeviceRegister(&consoleDevice);
    ConActivateConsole(0);

    return S_OK;
}

// STATUS ConSetActiveConsole(BYTE console)
// {
//   activeConsole = console;
//   pActiveConsole = &console[console];
//   return S_OK;
// }
STATUS ConpUpdateHardwareCursor(void);

/* Switch to the specified console */
STATUS ConActivateConsole(BYTE number)
{
    if(number >= NUM_CONSOLES)
        return S_FAIL;

    activeConsole = number;
    pActiveConsole = &console[activeConsole];

      
    // offset += CONSOLE_WIDTH;
    // DWORD offset = jvj
    DWORD offset = CONSOLE_SIZE * activeConsole / 2; // Divide by 2 because the low word is only 7 bits
    Debug("Switch to console %d at %d %d %d %d\n", number, offset, pActiveConsole, offset >> 8, offset & 0xff);
    IoWritePortByte(0x3d4, 12);
    IoWritePortByte(0x3d5, offset >> 8);
    IoWritePortByte(0x3d4, 13);
    IoWritePortByte(0x3d5, offset & 0xff);
    ConpUpdateHardwareCursor();

    return S_OK;
}

static int offset = 0;

/* Updates the position of the hardware cursor. Private. */
STATUS ConpUpdateHardwareCursor(void)
{
    WORD position = pActiveConsole->cursorY * CONSOLE_WIDTH + pActiveConsole->cursorX + offset;
    // Debug("%d %d %d\n", pActiveConsole->cursorY, pActiveConsole->cursorX, position);
    IoWritePortByte(0x3d4, 0x0e);
    IoWritePortByte(0x3d5, position >> 8);

    IoWritePortByte(0x3d4, 0x0f);
    IoWritePortByte(0x3d5, position);

    return S_OK;
}

void ScrollDown()
{
  Memcopy(pActiveConsole->buffer, pActiveConsole->buffer + (CONSOLE_WIDTH  * 2), (CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1) * 2));
  Memset(pActiveConsole->buffer + (CONSOLE_WIDTH * 2) * (CONSOLE_HEIGHT - 1), 0, CONSOLE_WIDTH * 2);
  // pActiveConsole->cursorY = 0;
  // pActiveConsole->cursorX = 0;

  
    // offset += CONSOLE_WIDTH;
    // DWORD offset = jvj
    // IoWritePortByte(0x3d4, 12);
    // IoWritePortByte(0x3d5, offset >> 8);
    // IoWritePortByte(0x3d4, 13);
    // IoWritePortByte(0x3d5, offset & 0xff);
    // ConpUpdateHardwareCursor();
}

STATUS ConGetCursorPosition(POINT* point)
{
    if(point == NULL)
        return S_FAIL;

    point->X = pActiveConsole->cursorX;
    point->Y = pActiveConsole->cursorY;
    return S_OK;
}

/* Moves the console's cursor to a new location */
STATUS ConMoveCursor(WORD newX, WORD newY)
{
    if(newX >= CONSOLE_WIDTH || newY >= CONSOLE_HEIGHT)
        return S_FAIL;

    pActiveConsole->cursorX = newX;
    pActiveConsole->cursorY = newY;

    ConpUpdateHardwareCursor();
    return S_OK;
}

// Moves the cursor one space back
STATUS ConBackspace(void)
{
    if(pActiveConsole->cursorX == 0)
    {
        if(pActiveConsole->cursorY == 0)
            return S_FAIL;

        pActiveConsole->cursorX = CONSOLE_WIDTH - 1;
        pActiveConsole->cursorY = pActiveConsole->cursorY - 1;
    }

    pActiveConsole->cursorX--;
    pActiveConsole->buffer[(pActiveConsole->cursorY * CONSOLE_WIDTH + pActiveConsole->cursorX) * 2] = ' ';

    ConpUpdateHardwareCursor();
    return S_OK;
}

/* Clears the console's screen */
STATUS ConClearScreen(void)
{
    int i;

    /* FIXME: use memset */
    for(i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT * 2; i += 2)
    {
        pActiveConsole->buffer[i] = ' ';
        pActiveConsole->buffer[i + 1] = pActiveConsole->color;
    }

    return S_OK;
}


/* Displays the requested string at the requested location */
STATUS ConDisplayString(const char* str, WORD x, WORD y)
{
    int loc;
    int newX, newY;
    loc = (y * CONSOLE_WIDTH + x) * 2;

    /*	Assert(str != NULL);
    	Assert(x > 0);
    	Assert(y > 0);*/

    if(str == NULL)
        return S_FAIL;
    POINT point; 
    ConGetCursorPosition(&point);

    while(*str != '\0')
    {
       if(*str == 10) {
          // If we hit a newline character, update position but don't write anything
          loc += (CONSOLE_WIDTH - point.X) * 2;
          point.Y++;
          point.X = 0;
        }
        else if(*str == 9) {
          // Tab character, write out tab
          BYTE tabWidth = 4; // TODO how to handle tab width?
          for(int i = 0; i < tabWidth; ++i) {
            pActiveConsole->buffer[loc++] = NULL;
            pActiveConsole->buffer[loc++] = 0x0;
          }
          point.X += 4;
        }
        else {
          // Otherwise, write the character to the console buffer
          pActiveConsole->buffer[loc++] = *str;
          pActiveConsole->buffer[loc++] = pActiveConsole->color;
          point.X++;
        }
       
        // If we're too far to the right, go to the start of the next line
        if(point.X >= CONSOLE_WIDTH) {
            point.X = 0;
            point.Y++;
        }

        // If we're too far down, scroll
        if(point.Y > 24) {
          ScrollDown();
          point.Y--;
          loc -= CONSOLE_WIDTH * 2;
        }

        // Update cursor position
        // ConMoveCursor(point.X, point.Y);
        // ScrollDown();
        *str++;
    }

    // TODO - calculate everything from loc instead of keeping track of position separately
    // newX = (loc % (CONSOLE_WIDTH * 2)) / 2;
    // newY = loc / (CONSOLE_WIDTH * 2);
    ConMoveCursor(point.X, point.Y);

    return S_OK;
}


/* Destroy console driver */
STATUS ConDestroy(void)
{
    return S_OK;
}

