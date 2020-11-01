
include ../config/Make.config


libs = syscall.o
objs = hello.o shell.o
apps = hello.exe shell.exe

VPATH = ../obj/



default: all


all: $(libs) $(objs) $(apps)


$(libs): %.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(objs): %.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(apps): $(objs) $(libs)
	$(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o hello.o -o hello.exe
	$(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o shell.o -o shell.exe



