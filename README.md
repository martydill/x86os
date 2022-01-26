x86OS
======

![Screenshot](/screenshot.gif "Screenshot")

x86OS is an x86-based hobby opearting system.

This project was originally started in around 2002 or 2003 and then abandoned for a long time. I resurrected it in around 2019
and got it building again on modern systems.

It is booted using a GRUB multiboot boot sector (this part wasn't written by me) on a floppy disk image. 

It is primarily designed to be run using VirtualBox.

## These things work in a "held together by duct tape" kind of way:
 - Keyboard input and console output
 - Readonly FAT12 floppy driver
 - Serial port driver for debug output
 - Preemptive multitasking with a simple scheduler
 - Basic sleep timer support
 - A tiny subset of POSIX syscalls (`read`, `write`, `open`, `stat`, `posix_spawn`, etc.)
 - Loading and executing user-mode ELF binaries
 - The beginnings of a standard I/O layer
 - The beginnings of a set of user-mode utilities like `ls`, `ps`, and so on
 - The beginnings of a ProcFS (`/proc`)

## These things do not exist yet:
- Proper memory management
- Writable filesystems
- A proper VFS layer
- Support for networking, sound, usb, video, hard disks, etc.
- Literally everything else

Development
===========


### Building and Running
The build environment is dockerized:
- `make docker-init`
	 - This will build the base docker image
- `make docker-build`
	 - This will compile x86os and produce a bootable floppy image in the root directory called `./floppy.img`
- `make run-qemu`
	- Launches x86os in QEMU using the floppy image. Requires `qemu-system-i386`.
- `make run-virtualbox`
	- Launches x86os in VirtualBox. Requires VirtualBox and a VirtualBox VM called `x86os`.

If you don't want to use the dockerized version you'll need these tools installed:
 - GCC
 - NASM
 - A version of LD that supports linker scripts, such as LLVM's ldd. The default ld in OS X does not support them.
 - mtools for manipulating the floppy image
