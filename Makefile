
# Top-level makefile


include config/Make.config

KERNEL = kernel0
SUBDIRS = boot kernel drivers

default: all


all: subdirs kernel1


subdirs:
	@for dir in $(SUBDIRS); do \
	$(MAKE) $(MAKECMDGOALS) -C $$dir; \
	done


kernel1: 
	$(LD) -T kernel.ls -nostdlib -nostartfiles -melf_i386 boot/boot.o boot/startup.o obj/*.o  -o bin/$(KERNEL)


clean:
	-rm -rf obj/*.o
	-rm bin/$(KERNEL)
	-rm boot/*.o



