
include ../config/Make.config


libs = syscall.o
objs = hello.o
apps = hello.exe

VPATH = ../obj/



default: all


all: $(libs) $(objs) $(apps)


$(libs): %.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(objs): %.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(apps): hello.o syscall.o
	$(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o hello.o -o hello.exe



