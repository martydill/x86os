
include ../config/Make.config


objs = Keyboard.o Mouse.o Floppy.o Console.o SerialPort.o pci.o


VPATH = ../obj/


default: all


all: $(objs)


$(objs): %.o: %.c
	$(CC) $(CFLAGS) -c $<  -o ../obj/$@

