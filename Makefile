
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
	cp base_images/floppy.img ./floppy.img
	mcopy -o -i ./floppy.img bin/kernel0 ::
	mcopy -o -i ./floppy.img apps/hello.exe ::
	mcopy -o -i ./floppy.img README.md ::README.md
	rm -f ./output.txt
	vboxmanage startvm fizzos
	sleep 3
	vboxmanage controlvm fizzos keyboardputscancode 1c 9c
	tail -f ./output.txt



clean:
	-rm -rf obj/*.o
	-rm bin/$(KERNEL)
	-rm boot/*.o



