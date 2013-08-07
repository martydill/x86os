
include ../config/Make.config

VPATH = ../obj/

default: all

all: startup.o boot.o

startup.o: startup.c 
	$(CC) $(CFLAGS) -c startup.c -o startup.o


boot.o: boot.s
	$(AS) $(ASFLAGS) -f elf -o boot.o boot.s

