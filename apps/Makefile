
include ../config/Make.config


libs = syscall.o
objs = hello.o
apps = hello.exe

VPATH = ../obj/



default: all


all: $(libs) $(objs) $(apps)


$(libs): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


$(objs): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(apps): %.exe: %.o
	$(LD) -T apps.ls -nostdlib -melf_i386 $< $(libs) -o $@



