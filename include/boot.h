
/*
 * boot.h
 * Contains all the GRUB multiboot info
 */

#ifndef BOOT_H
#define BOOT_H

/* The symbol table for a.out. */
typedef struct {
  unsigned long tabsize;
  unsigned long strsize;
  unsigned long addr;
  unsigned long reserved;
} AOutSymbolTable;

/* The section header table for ELF. */
typedef struct {
  unsigned long num;
  unsigned long size;
  unsigned long addr;
  unsigned long shndx;
} ElfSectionHeaderTable;

/* The Multiboot information.  */
typedef struct {
  unsigned long flags;
  unsigned long mem_lower;
  unsigned long mem_upper;
  unsigned long boot_device;
  unsigned long cmdline;
  unsigned long mods_count;
  unsigned long mods_addr;

  union {
    AOutSymbolTable aout_sym;
    ElfSectionHeaderTable elf_sec;
  } u;

  unsigned long mmap_length;
  unsigned long mmap_addr;
} MultibootInfo;

#endif
