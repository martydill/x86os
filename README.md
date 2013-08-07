FizzOS
======
FizzOS is an experimental sort-of-OS that I hacked together many years ago. It doesn't do a whole lot, but it does have basic keyboard, console, and serial support, and the beginnings of a floppy disk driver.

It is primarily designed to be run using the Bochs emulator.
It is booted using a GRUB multiboot boot sector (this part wasn't written by me) on a floppy disk image. 

![Screenshot](/screenshot.png "Screenshot")

Setup:
* Install gcc
* Install nasm (sudo apt-get install nasm)
* Install bochs (sudo apt-get install bochs bochs-x)
(you may also have to install bochs-sdl and change display_library in your bochs config file from x to sdl)
* Run 'make' to compile.
* Run ./update as root to mount the flopy image as a loopback device, copy the newly built kernel to it, and unmount the floppy image.
* Run bochs!

Using the included config file, bochs will dump serial output to a file called serial.txt. You can monitor its contents with the command 'tail -f serial.txt'
