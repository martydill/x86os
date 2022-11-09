#define DEBUG 1
#include <kernel.h>
#include <interrupt.h>
#include <io.h>
#include <console.h>

const char* exceptionStrings[] = {"Divide by zero",
                                  "Debug/Trap",
                                  "NMI",
                                  "Debug Breakpoint",
                                  "Integer Overflow",
                                  "Bounds Check",
                                  "Invalid Opcode",
                                  "Device Not Available",
                                  "Double Fault",
                                  "Coprocessor Segment Overrun",
                                  "Invalid TSS",
                                  "Segment Not Present",
                                  "Stack Segment",
                                  "General Protection Fault",
                                  "Page Fault",
                                  "Reserved",
                                  "FPU Error",
                                  "Alignment Check",
                                  "18",
                                  "19",
                                  "20",
                                  "21",
                                  "22",
                                  "23",
                                  "24",
                                  "25",
                                  "26",
                                  "27",
                                  "28",
                                  "29",
                                  "30",
                                  "31",
                                  "something",
                                  "timer",
                                  "keyboard",
                                  "redirect",
                                  "comm2",
                                  "comm1",
                                  "sound card",
                                  "floppy",
                                  "parallel",
                                  "24",
                                  "25"};

IDTEntry idtTable[256];
IDTPointer idtPointer;

/*
 * Installs a handler for the specified interrupt)
 * Handler must be a naked function (i.e. ASM)
 */
void AddInterruptHandler(int number, void* handler, BYTE accessMode) {
  Assert(number >= 0 && number < 256);
  Assert(handler != NULL);

  idtTable[number].Low = (unsigned long)handler & 0xFFFF;
  idtTable[number].High = ((unsigned long)handler >> 16) & 0xFFFF;
  idtTable[number].Selector = 0x08;
  idtTable[number].Unused = 0;
  idtTable[number].AccessMode = accessMode;
}

void AddUserModeHandler(int number, void* handler) {
  AddInterruptHandler(number, handler, RING_3_INTERRUPT);
}

void AddKernelModeHandler(int number, void* handler) {
  AddInterruptHandler(number, handler, RING_0_INTERRUPT);
}

/* fixme */
void irq_remap(void) {
  IoWritePortByte(0x20, 0x11);
  IoWritePortByte(0xA0, 0x11);
  IoWritePortByte(0x21, 0x20);
  IoWritePortByte(0xA1, 0x28);
  IoWritePortByte(0x21, 0x04);
  IoWritePortByte(0xA1, 0x02);
  IoWritePortByte(0x21, 0x01);
  IoWritePortByte(0xA1, 0x01);
  IoWritePortByte(0x21, 0x0);
  IoWritePortByte(0xA1, 0x0);
}

#define MAX_IRQ 256
void* irqHandlers[MAX_IRQ];

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8 and 70, as configured by default */

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20 /* End - of - interrupt command code */

#define ICW1_ICW4 0x01      /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

/*
  arguments:
    offset1 - vector offset for master PIC
      vectors on the master become offset1..offset1+7
    offset2 - same for slave PIC: offset2..offset2+7
 */
void remap_pics(int offset1, int offset2) {
  BYTE a1, a2;

  a1 = IoReadPortByte(PIC1_DATA);
  a2 = IoReadPortByte(PIC2_DATA);

  IoWritePortByte(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);

  IoWritePortByte(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);

  IoWritePortByte(PIC1_DATA, offset1);

  IoWritePortByte(PIC2_DATA, offset2);

  IoWritePortByte(PIC1_DATA, 4);
  IoWritePortByte(PIC2_DATA, 2);

  IoWritePortByte(PIC1_DATA, ICW4_8086);

  IoWritePortByte(PIC2_DATA, ICW4_8086);
  IoWritePortByte(PIC1_DATA, a1);
  IoWritePortByte(PIC2_DATA, a2);
}

/* Set up all of our interrupt handlers */
void InitializeIDT(void) {
  /*irq_remap();*/
  remap_pics(32, 40);

  /* Everything starts out zeroed out */
  Memset((void*)&idtTable, 0, 256 * sizeof(struct IDTEntry_S));
  Memset((void*)&irqHandlers, 0, MAX_IRQ * sizeof(void*));

  // Unmask all interrupts so that everything can fire
  IoWritePortByte(PIC1_DATA, 0x00);
  IoWritePortByte(PIC2_DATA, 0x00);

  /* Standard exceptions */
  AddKernelModeHandler(0, _interrupt0);
  AddKernelModeHandler(1, _interrupt1);
  AddKernelModeHandler(2, _interrupt2);
  AddKernelModeHandler(3, _interrupt3);
  AddKernelModeHandler(4, _interrupt4);
  AddKernelModeHandler(5, _interrupt5);
  AddKernelModeHandler(6, _interrupt6);
  AddKernelModeHandler(7, _interrupt7);
  AddKernelModeHandler(8, _interrupt8);
  AddKernelModeHandler(9, _interrupt9);
  AddKernelModeHandler(10, _interrupt10);
  AddKernelModeHandler(11, _interrupt11);
  AddKernelModeHandler(12, _interrupt12);
  AddKernelModeHandler(13, _interrupt13);
  AddKernelModeHandler(14, _interrupt14);
  AddKernelModeHandler(15, _interrupt15);

  AddKernelModeHandler(16, _reservedexceptionhandler);
  AddKernelModeHandler(17, _reservedexceptionhandler);
  AddKernelModeHandler(18, _reservedexceptionhandler);
  AddKernelModeHandler(19, _reservedexceptionhandler);
  AddKernelModeHandler(20, _reservedexceptionhandler);
  AddKernelModeHandler(21, _reservedexceptionhandler);
  AddKernelModeHandler(22, _reservedexceptionhandler);
  AddKernelModeHandler(23, _reservedexceptionhandler);
  AddKernelModeHandler(24, _reservedexceptionhandler);
  AddKernelModeHandler(25, _reservedexceptionhandler);
  AddKernelModeHandler(26, _reservedexceptionhandler);
  AddKernelModeHandler(27, _reservedexceptionhandler);
  AddKernelModeHandler(28, _reservedexceptionhandler);
  AddKernelModeHandler(29, _reservedexceptionhandler);
  AddKernelModeHandler(30, _reservedexceptionhandler);
  AddKernelModeHandler(31, _reservedexceptionhandler);

  /* Standard interrupt handlers */
  AddKernelModeHandler(32, _interrupt32);
  AddKernelModeHandler(33, _interrupt33);
  AddKernelModeHandler(34, _interrupt34);
  AddKernelModeHandler(35, _interrupt35);
  AddKernelModeHandler(36, _interrupt36);
  AddKernelModeHandler(37, _interrupt37);
  AddKernelModeHandler(38, _interrupt38);
  AddKernelModeHandler(39, _interrupt39);
  AddKernelModeHandler(40, _interrupt40);
  AddKernelModeHandler(41, _interrupt41);
  AddKernelModeHandler(42, _interrupt42);
  AddKernelModeHandler(43, _interrupt43);
  AddKernelModeHandler(44, _interrupt44);
  AddKernelModeHandler(45, _interrupt45);
  AddKernelModeHandler(46, _interrupt46);
  AddKernelModeHandler(47, _interrupt47);
  AddKernelModeHandler(48, _interrupt48);
  AddKernelModeHandler(112, _interrupt112);

  // Syscall handler is in user mode
  AddUserModeHandler(128, _interrupt128);

  idtPointer.Limit = sizeof(struct IDTEntry_S) * 256 - 1;
  idtPointer.Base = (DWORD)&idtTable;

  __asm__ __volatile__("lidt %0" : "=m"(idtPointer));

  return;
}

/* Generic handler for all interrupts/exceptions */
void KeExceptionHandler(Registers* registers) {
  KeDisableInterrupts();
  Assert(registers != NULL);

  if (registers->interruptNumber < MAX_INTERRUPT) {
    /* Exception */
    Debug("Caught exception %u: %s, error: %u", registers->interruptNumber,
          exceptionStrings[registers->interruptNumber], registers->errorCode);
    if (registers->interruptNumber == 13) {
      if (registers->errorCode & (1 << 0)) {
        Debug("External\n");
      }
      if (registers->errorCode & (1 << 1)) {
        Debug("Bit 1 set\n");
      }
      if (registers->errorCode & (1 << 2)) {
        Debug("Bit 2 set\n");
      }
    }

    if (registers->interruptNumber != 14) {
      KePanic(registers);
    }
  } else if (registers->interruptNumber == 999) {
    Debug("Caught reserved exception");
    // KPrint("Caught reserved exception");
  }

  /* remapped irq */
  // irq = registers->interruptNumber;// - MAX_INTERRUPT;
  // Assert(irq < MAX_IRQ && irq < sizeof(irqHandlers));
  if (registers->interruptNumber != 32) {
    Debug("Interrupt %d\n", registers->interruptNumber);
  }
  void (*handler)() = irqHandlers[registers->interruptNumber];
  if (handler == NULL) {
    Debug("NULL handler for interrupt %d\r\n", registers->interruptNumber);
  } else {
    handler(registers);
  }

  /* If this is an IRQ handler, tell the PIC we are done */
  if (registers->interruptNumber >= MAX_INTERRUPT) {
    if (registers->interruptNumber - MAX_INTERRUPT >= SECOND_PIC_IRQ)
      IoWritePortByte(0xA0, 0x20);

    IoWritePortByte(0x20, 0x20);
  }
  //}
  KeEnableInterrupts();
}

/* Assigns the specified handler to the given irq */
STATUS InstallIrqHandler(unsigned int irq, InterruptHandler handler) {
  Debug("Installing irq %d handler at %d\r\n", irq, (int)handler);
  return InstallInterruptHandler(irq + MAX_INTERRUPT, handler);
}

/* Clears the handler for the given irq */
STATUS RemoveIrqHandler(unsigned int irq) {
  Debug("Removing irq %d handler\r\n", irq);
  return RemoveInterruptHandler(irq + MAX_INTERRUPT);
}

/* Assigns the specified handler to the given interrupt */
STATUS InstallInterruptHandler(unsigned int interrupt,
                               InterruptHandler handler) {
  Assert(interrupt < MAX_IRQ);
  Assert(handler != NULL);
  Assert(irqHandlers[interrupt] == NULL);

  Debug("Installing interrupt %d handler at %d\r\n", interrupt, (int)handler);
  irqHandlers[interrupt] = handler;
  return S_OK;
}

/* Clears the handler for the given interrupt */
STATUS RemoveInterruptHandler(unsigned int interrupt) {
  Assert(interrupt < MAX_IRQ);
  Assert(irqHandlers[interrupt] != NULL);

  Debug("Removing interrupt %d handler\r\n", interrupt);
  irqHandlers[interrupt] = NULL;
  return S_OK;
}
