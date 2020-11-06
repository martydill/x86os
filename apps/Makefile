
include ../config/Make.config


libs = syscall.o
objs = hello.o #fshell.o mount.o
apps = hello #shell mount

VPATH = ../obj/
OUTPUT_DIR = bin/


default: all


all: boot.o string.o $(libs) $(objs) $(apps)

string.o: ../kernel/string.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

boot.o: boot.s
	$(AS) $(ASFLAGS) -f elf -o boot.o boot.s

$(libs): %.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(objs): %.o: %.c 
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(apps): $(objs) $(libs) boot.o string.o
	$(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o boot.o hello.o ../obj/string.o -o $(OUTPUT_DIR)hello
	# $(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o boot.o shell.o -o $(OUTPUT_DIR)shell
	# $(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o boot.o mount.o -o $(OUTPUT_DIR)mount



