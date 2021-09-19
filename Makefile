
# Top-level makefile


include config/Make.config

KERNEL = kernel0
SUBDIRS = boot kernel drivers apps

.PHONY: subdirs $(SUBDIRS)

default: all


all: subdirs kernel1


subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) $(MAKECMDGOALS) -C $@


kernel1: subdirs
	$(LD) -T kernel.ls -nostdlib -melf_i386 boot/boot.o boot/startup.o obj/*.o  -o bin/$(KERNEL)

run:
	vboxmanage controlvm x86os poweroff | true
	sleep 1
	cp base_images/floppy.img ./floppy.img
	mcopy -o -i ./floppy.img bin/kernel0 ::
	mcopy -o -i ./floppy.img apps/bin/* ::
	mcopy -o -i ./floppy.img README.md ::README.md
	mcopy -o -i ./floppy.img Makefile ::makefile
	mmd -i ./floppy.img usr
	mcopy -o -i ./floppy.img ./test.txt ::usr/test.txt
	rm -f ./output.txt
	vboxmanage startvm x86os
	sleep 1 
	vboxmanage controlvm x86os keyboardputscancode 1c 9c
	tail -f ./output.txt

clean:
	vboxmanage controlvm x86os poweroff | true
	-rm -rf obj/*.o
	-rm bin/$(KERNEL)
	-rm boot/*.o
	-rm -rf apps/bin apps/*.o


format:
	find ./ -name *.c -o -name *.h | xargs clang-format -i
