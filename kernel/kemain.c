
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
#include <fs.h>
#include <syscall.h>

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
void KePanic(Registers* registers)
{
    ConMoveCursor(0,20);
    KPrint("Kernel panic, interrupt #%d\n", registers->interruptNumber);
    KPrint("EAX: %u    EBX: %u   ECX: %u   EDX: %u\n", registers->eax, registers->ebx, registers->ecx, registers->edx);
    KPrint("EDI: %u    ESI: %u   ESP: %u   EIP: %u\n", registers->edi, registers->esi, registers->esp, registers->eip);
    KPrint("Done dumping\n");

    KeHalt();
}

void ShellStart();
void IdleLoop();

void KeSysCallHandler(Registers* registers)
{
   Debug("Syscall handler %u %u %u %u %u %u %u %u %u\n",  registers->eax, registers->ebx, registers->ecx, registers->edx, registers->esi, registers->edi, registers->ebp, registers->esp, registers->eip);
   if(registers->eax == SYSCALL_EXIT) {
     SyscallExit(registers);
   }
   else if(registers->eax == SYSCALL_KPRINT) {
      SyscallKPrint(registers->ebx);
     registers->eax = 0;
   }
   else if(registers->eax == SYSCALL_OPEN) {
    int fd  = SyscallOpen(registers->ebx, registers->ecx);
    registers->eax = fd;
   }
   else if(registers->eax == SYSCALL_READ) {
    SyscallRead(registers);
   }
   else if(registers->eax == SYSCALL_WRITE) {
     SyscallWrite(registers);
   }
   Debug("Done syscall handler, returning to %u for stack %u\n", registers->eip, registers->esp);
}

extern void _jump_usermode();

/* Start here ... */
int KeMain(MultibootInfo* bootInfo)
{
    unsigned int amount;
    char* buf;

    InitializeGDT();
    InitializeIDT();
    MMInitializePaging();
    DeviceInit();

    ConInit();
    ConClearScreen();

    KPrint("FizzOS kernel 0.0.2\n");

    KPrint("Initializing timer...\n");
    TimerInit();

    KPrint("Initializing serial port...\n");
    SerialPortInit();

    KPrint("Initializing DMA...\n");
    DmaInit();

    KPrint("Initializing FPU...\n");
    setup_x87_fpu();

    KPrint("Initializing processes...\n");
    ProcessInit();

    KPrint("Initializing keyboard...\n");
    KbInit();
    KeEnableInterrupts();

    KPrint("Initializing floppy drives...\n");
    FloppyInit();

    KPrint("Initializing PCI ...\n");
    PciInit();

    InstallInterruptHandler(0x80, KeSysCallHandler);
    Test_String();

    KeDisableInterrupts();
    KPrint("Initializing VFS\n");
    FSInit();
  // ShellStart();
    
    CreateProcess(IdleLoop, "Idle00", 0, "idle");
    CreateProcess(ShellStart, "Shell1", 255, "shell");
    KeEnableInterrupts();
    // CreateProcess(ShellStart, "Shell2", 0);
    // CreateProcess(ShellStart, "Shell3", 0);
    // CreateProcess(ShellStart, "Shell4", 0);
    // CreateProcess(ShellStart, "Shell5", 0);
    // CreateProcess(ShellStart, "Shell6", 0);
    // CreateProcess(ShellStart, "Shell7", 0);
    // CreateProcess(ShellStart, "Shell8", 0);


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
    char buffer[8192]; // TODO make dynamic
    POINT point;

    va_start(args, format);
    DoSprintf(sizeof(buffer), buffer, format, args);
    va_end(args);

    ConGetCursorPosition(&point);
    ConDisplayString(buffer, point.X, point.Y);

    return;
}

void KResetCursor()
{
    xPos = 0;
    yPos = 0;
}
