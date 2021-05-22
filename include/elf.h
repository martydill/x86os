
#ifndef ELF_H
#define ELF_H

typedef struct {
	BYTE e_ident[16];
	WORD e_type;
	WORD e_machine;
	DWORD e_version;
	DWORD e_entry;
	DWORD e_phoff;
	DWORD e_shoff;
	DWORD e_flags;
	WORD e_ehsize;
	WORD e_phentsize;
	WORD e_phnum;
  WORD e_shentsize;
	WORD e_shnum;
	WORD e_shstrndx;
}  __attribute__ ((packed)) ELFHeader;


typedef struct {
	DWORD p_type;
	DWORD	p_offset;
	DWORD	p_vaddr;
	DWORD	p_paddr;
	DWORD	p_filesz;
	DWORD	p_memsz;
	DWORD	p_flags;
	DWORD	p_align;
}  __attribute__ ((packed)) ELFProgramHeader;


typedef struct {
	DWORD	sh_name;
	DWORD	sh_type;
	DWORD	sh_flags;
	DWORD	sh_addr;
	DWORD	sh_offset;
	DWORD	sh_size;
	DWORD	sh_link;
	DWORD	sh_info;
	DWORD	sh_addralign;
	DWORD	sh_entsize;
} __attribute__ ((packed)) ELFSectionHeader;

// Section types
#define SHT_NULL 0x0
#define SHT_PROGBITS 	0x1
#define SHT_SYMTAB 	0x2 
#define SHT_STRTAB 	0x3
#define SHT_RELA 	0x4
#define SHT_HASH 	0x5
#define SHT_DYNAMIC 	0x6
#define SHT_NOTE 	0x7
#define SHT_NOBITS 	0x8
#define SHT_REL 	0x9
#define SHT_SHLIB 	0x0A
#define SHT_DYNSYM 	0x0B
#define SHT_INIT_ARRAY 0x0E
#define SHT_FINI_ARRAY 	0x0F
#define SHT_PREINIT_ARRAY 	0x10
#define SHT_GROUP 	0x11
#define SHT_SYMTAB_SHNDX 	0x12
#define SHT_NUM 	0x13
#define SHT_LOOS 	0x60000000 

// Section flags
#define	SHF_WRITE 0x1
#define	SHF_ALLOC 	0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 	0x20
#define SHF_INFO_LINK 	0x40
#define SHF_LINK_ORDER 	0x80
#define SHF_OS_NONCONFORMING 	0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_MASKPROC 	0xf0000000 
#define SHF_ORDERED 0x4000000 
#define SHF_EXCLUDE 0x8000000 


DWORD ELFParseFile(BYTE* data, char* processName, char* commandLine);

#endif

