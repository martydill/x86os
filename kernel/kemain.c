
/*
* KeMain.c
* Kernel entry point
*/

#include <kernel.h>
#include <boot.h>
#include <console.h>
#include <mm.h>
#include <interrupt.h>
#include <gdt.h>
#include <timer.h>
#include <keyboard.h>
#include <memory.h>
#include <device.h>
#include <fpu.h>
#include <serialport.h>
#include <pci.h>
#include <process.h>

/* Enable interrupts */
void KeEnableInterrupts(void)
{
    __asm__ ("sti");
    return;
}


/* Disable interrutps */
void KeDisableInterrupts(void)
{
    __asm__ ("cli");
    return;
}


/* Halts the system */
void KeHalt(void)
{
    KeDisableInterrupts();
    while(1)
        __asm__ ("hlt");
}


/* Dumps registers to the screen and halts */
void KePanic(void)
{
    int eax, ebx, ecx, edx;
    char buffer[128];

    __asm__ __volatile__ ("":"=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx));

    ConDisplayString("Kernel panic", 0, 0);
    ConDisplayString("Contents of CPU registers: ", 0, 1);

    Sprintf(128, buffer, "EAX: %d    EBX: %d   ECX: %d   EDX: %d ", eax, ebx, ecx, edx);
    ConDisplayString(buffer, 0, 2);

    KeHalt();
}

void ShellStart();
void IdleLoop();

/* Start here ... */
int KeMain(MultibootInfo* bootInfo)
{
    unsigned int amount;
    char* buf;

    DeviceInit();

    ConInit();
    ConClearScreen();

    KPrint("FizzOS kernel 0.0.2\n");

    KPrint("Initializing serial port...\n");
    SerialPortInit();

    KPrint("Initializing GDT...\n");
    InitializeGDT();

    KPrint("Initializing IDT...\n");
    InitializeIDT();

    KPrint("Initializing DMA...\n");
    DmaInit();

    KPrint("Initializing FPU...\n");
    setup_x87_fpu();

    KPrint("Initializing processes...\n");
    ProcessInit();

    KPrint("Initializing timer...\n");
    TimerInit();

    KPrint("Initializing keyboard...\n");
    KbInit();
    KeEnableInterrupts();

    KPrint("Initializing floppy drives...\n");
    FloppyInit();

    KPrint("Initializing PCI ...\n");
    PciInit();

  // ShellStart();
    // MMInitializePaging();
    CreateProcess(IdleLoop, "IdleLoop");
    CreateProcess(ShellStart, "Shell");

    IdleLoop();

    return 0;
}

void IdleLoop(){
  while(1) {
    __asm__("hlt");
  }
}
volatile int xPos = 0;
volatile int yPos = 0;

void KPrint(const char* format, ...)
{
    va_list args;
    char buffer[256];
    POINT point;

    va_start(args, format);
    DoSprintf(sizeof(buffer), buffer, format, args);
    va_end(args);

    if(buffer[Strlen(buffer) - 1] == '\n')
    {
        buffer[Strlen(buffer) - 1] = ' ';
        ConGetCursorPosition(&point);
        ConDisplayString(buffer, point.X, point.Y);
        point.Y++;
        point.X = 0;
        if(point.Y > 24) {
            ScrollDown();
            point.Y--;
        }
        ConMoveCursor(point.X, point.Y);
    }
    else
    {
        ConGetCursorPosition(&point);
        ConDisplayString(buffer, point.X, point.Y);
        point.X += Strlen(buffer);
    }

    return;
}

void KResetCursor()
{
    xPos = 0;
    yPos = 0;
}
