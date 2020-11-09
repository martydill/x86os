
include ../config/Make.config


libs = syscall.o

OUTPUT_DIR = bin

SRCS := $(wildcard *.c)
EXES := $(OUTPUT_DIR)/hello $(OUTPUT_DIR)/mount $(OUTPUT_DIR)/cat #$(patsubst %.c, $(OUTPUT_DIR)/%.exe, $(SRCS))

objs = hello.o mount.o
apps = hello mount

VPATH = ../obj/


default: all


# all: boot.o string.o $(libs) $(objs) $(apps)

string.o: ../kernel/string.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

boot.o: boot.s
	$(AS) $(ASFLAGS) -f elf -o boot.o boot.s

$(libs): %.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -g0 -nostartfiles -c $< -o $@

$(OUTPUT_DIR)/%: %.o boot.o string.o syscall.o
	$(LD) -T apps.ls -nostdlib -melf_i386 -static syscall.o boot.o $< ../obj/string.o -o $@



all: $(EXES)

clean:
	rm -rf *.o
	rm -rf bin/*