
include ../config/Make.config

libs = syscall.o

OUTPUT_DIR = bin

SRCS := $(wildcard *.c)
EXES := $(OUTPUT_DIR)/cat $(OUTPUT_DIR)/shell $(OUTPUT_DIR)/idle $(OUTPUT_DIR)/ls $(OUTPUT_DIR)/ps $(OUTPUT_DIR)/sleep $(OUTPUT_DIR)/echo  $(OUTPUT_DIR)/kill #$(patsubst %.c, $(OUTPUT_DIR)/%.exe, $(SRCS))


default: all

syscall.o: syscall.c
	$(CC) $(CFLAGS) -g0 -c $< -o syscall.o

string.o: ../kernel/string.c
	$(CC) $(CFLAGS) -g0 -c $< -o $@

boot.o: boot.s
	$(AS) $(ASFLAGS) -f elf -o boot.o boot.s

%.o: %.c
	$(CC) $(CFLAGS) -g0 -c $< -o $@

$(OUTPUT_DIR)/%: %.o boot.o string.o syscall.o
	mkdir -p $(OUTPUT_DIR)
	$(LD) -T apps.ls -e _start -nostdlib -melf_i386 -static syscall.o boot.o $< ../obj/string.o -o $@

all: $(EXES)

clean:
	rm -rf *.o
	rm -rf bin/*