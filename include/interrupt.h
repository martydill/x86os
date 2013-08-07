
#ifndef INTERRUPT_H
#define INTERRUPT_H


/* Note: Interrupts clear IF bit of EFLAGS. Traps don't. */
#define RING_0_TRAP			0x8f
#define RING_0_INTERRUPT	0x8e
#define RING_3_TRAP			0xef
#define RING_3_INTERRUPT	0xee


/* Entry in IDT table */
struct IDTEntry_S
{
    WORD Low;
    WORD Selector;
    BYTE Unused;
    BYTE AccessMode;
    WORD High;
} __attribute__((packed));

typedef struct IDTEntry_S IDTEntry;


/* Magic pointer to IDT table */
/* fixme: merge idt and gdt pointers */
struct IDTPointer_S
{
    WORD Limit;
    DWORD Base;
} __attribute__ ((packed));

typedef struct IDTPointer_S IDTPointer;

/* Interrupt handlers defined in Interrupt.s */
extern void _isr();
extern void _interrupt0();
extern void _interrupt1();
extern void _interrupt2();
extern void _interrupt3();
extern void _interrupt4();
extern void _interrupt5();
extern void _interrupt6();
extern void _interrupt7();
extern void _interrupt8();
extern void _interrupt9();
extern void _interrupt10();
extern void _interrupt11();
extern void _interrupt12();
extern void _interrupt13();
extern void _interrupt14();
extern void _interrupt15();
extern void _interrupt32();
extern void _interrupt33();
extern void _interrupt34();
extern void _interrupt35();
extern void _interrupt36();
extern void _interrupt37();
extern void _interrupt38();
extern void _interrupt39();
extern void _interrupt40();
extern void _interrupt41();
extern void _interrupt112();
extern void _reservedexceptionhandler();

/*defines what the stack looks like after an ISR was running */
/* Must match order in Interrupt.s */
typedef struct
{
    unsigned int gs;
    unsigned int fs;
    unsigned int es;
    unsigned int ds;

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int interruptNumber;
    unsigned int errorCode;

    unsigned int eip;
    unsigned int cs;
    unsigned int flags;
    unsigned int userEsp;
    unsigned int ss;
} Registers;

void InitializeIDT(void);

typedef void(*InterruptHandler) (Registers* registers);

STATUS InstallIrqHandler(unsigned int irq, InterruptHandler handler);
STATUS RemoveIrqHandler(unsigned int irq);

STATUS InstallInterruptHandler(unsigned int interrupt, InterruptHandler handler);
STATUS RemoveInterruptHandler(unsigned int interrupt);

#define SECOND_PIC_IRQ		8
#define MAX_INTERRUPT		32

#endif
