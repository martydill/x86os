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

 ## Prerequisites:
  - Mac OS X (it might work on other platforms but I haven't tried it)
  - GCC
  - NASM
  - LLVM (we use ldd, the LLVM linker)
  - VirtualBox (not strictly required but it's what I use)

### Building and Running
- To build: 
	 - `make`
	 - This will build the source tree and produce a bootable floppy image at `./floppy.img`

- To run:
  
   - Create a VirtualBox VM named `x86os`. Point it to that floppy image. Point the serial port to a file (I use `serial.txt`) for easier debugging
   - Run `make run`. This will start virtualbox and launch x86os.
   - Tail the serial port file with `tail -f serial.txt` for extra log output
