
/*
* Keyboard.c
* Keyboard driver
*/

#include <kernel.h>
#include <keyboard.h>
#include <interrupt.h>
#include <io.h>
#include <console.h>
#include <device.h>

#define KB_BUFFER_SIZE	256

int controlKeyState = 0;

#define CONTROL_KEY_SHIFT   (1 << 0)

char kbBuffer[KB_BUFFER_SIZE];
int kbBufferPosition = 0;
int startPos = 0;
int endPos = 0;

#define KEY_RELEASED_MASK	(0x80)

#define ESC  27
#define BACKSPACE  '\b'
#define TAB  '\t'
#define ENTER  '\n'
#define CTRL  0
#define UNKNOWN41  0
#define LSHIFT  0
#define UNKNOWN43  0
#define RSHIFT  0
#define UNKNOWN55  0
#define LALT  0
#define SPACEBAR  ' '

char keyCodes[128] =
{
    0, ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', BACKSPACE,
    TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', ENTER,
    CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', UNKNOWN41,
    LSHIFT, UNKNOWN43, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', RSHIFT,
    UNKNOWN55, LALT, SPACEBAR /* fixme add rest of keys */
};

char shiftKeyCodes[128] =
{
    0, ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', BACKSPACE,
    TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', ENTER,
    CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '|', UNKNOWN41,
    LSHIFT, UNKNOWN43, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', RSHIFT,
    UNKNOWN55, LALT, SPACEBAR /* fixme add rest of keys */
};
const BYTE functionKeyStart = 59;
const BYTE functionKeyEnd = functionKeyStart + 11;

BOOL IsNonPrintableKey(BYTE key) {
  return key >= functionKeyStart && key <= functionKeyEnd;
}

void AddKeyToBuffer(char key)
{
    if(endPos == KB_BUFFER_SIZE)
        endPos = 0;

    kbBuffer[endPos] = key;
    ++endPos;
}

void KeyboardHandler(Registers* registers)
{
    char c;
    BYTE key = IoReadPortByte(KB_DATA_REGISTER);
    if(key & KEY_RELEASED_MASK)
    {
        key = key - 128;
        // Keyboard driver-level support for switching between consoles
        if(key >= functionKeyStart && key <= functionKeyEnd) {
          // Switch consoles
          BYTE console = key - functionKeyStart;
          ConActivateConsole(console);
          ProcessSetForegroundProcessId(console + 1);
          return;
        }
        
        c = keyCodes[key];
        if(c == LSHIFT)
            controlKeyState &= ~CONTROL_KEY_SHIFT;
        /*AddKeyToBuffer(c);*/
    }
    else
    {
        if(IsNonPrintableKey(key)) {
          return;
        }

        c = keyCodes[key];

        if(c == LSHIFT)
        {
            controlKeyState |= CONTROL_KEY_SHIFT; // fixme - might want to add this to buffer and handle shift at a higher level...
            return;
        }

        if(controlKeyState & CONTROL_KEY_SHIFT)
            c = shiftKeyCodes[key];

        AddKeyToBuffer(c);
    }

    /*AddKeyToBuffer(c);*/
}

Device kbDevice;

/* The keyboard device's read handler */
// fixme - read numBytes
STATUS KbRead(char* buffer, int numBytes)
{
    if(startPos == endPos)
    {
        return S_FAIL;
    }
    else
    {
        buffer[0] = kbBuffer[startPos];
        startPos++;
        if(startPos == KB_BUFFER_SIZE)
            startPos = 0;
    }
    return S_OK;
}

/* Initialize keyboard device */
STATUS KbInit(void)
{
    InstallIrqHandler(KB_IRQ, KeyboardHandler);

    kbDevice.Name = "Keyboard";
    kbDevice.Read = KbRead;
    kbDevice.Status = 0;
    kbDevice.Status |= DEVICE_CAN_READ;
    kbDevice.Status |= DEVICE_OPEN;

    DeviceRegister(&kbDevice);
    return S_OK;
}


/* Destroy keyboard device */
STATUS KbDestroy(void)
{
    RemoveIrqHandler(KB_IRQ);
    DeviceUnregister(&kbDevice);
    return S_OK;
}
