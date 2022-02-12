
include ../config/Make.config

objs = kemain.o elf.o gdt.o timer.o interrupt.o io.o dbgmain.o string.o mm.o memory.o assert.o device.o dma.o fat.o scheduler.o fpu.o fs.o syscall.o tss.o test.o path.o

asms = interrupt_backend.o

VPATH = ../obj/

default: all

all: $(objs) $(asms)


$(objs): %.o: %.c
	$(CC) $(CFLAGS) -c $<  -o ../obj/$@


$(asms): %.o: %.s
	$(AS) $(ASFLAGS) -f elf -o ../obj/$@ $<
	
