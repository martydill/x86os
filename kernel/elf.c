#include <kernel.h>
#include <elf.h>


STATUS ELFParseFile(BYTE* data, char* processName, char* commandLine)
{
  if(data == NULL) 
  {
    return S_FAIL;
  }

  ELFHeader* header = (ELFHeader*)data;
  KPrint("Running %s\n", processName);
  KPrint("Ident: %s\n", header->e_ident);
  KPrint("Type: %d\n", header->e_type);
  KPrint("Entry: %u\n", header->e_entry);

  void* addr = 1024 * 1024 * 8;

 
   Debug("Section Headers: %u (%u bytes, offset %u)\n", header->e_shnum, header->e_shentsize, header->e_shoff);
  for(int i = 0; i < header->e_shnum; ++i) {
    ELFSectionHeader* sectionHeader= data + header->e_shoff + (i * header->e_shentsize);
    Debug("\tType: %u  Address: %u\n  Offset: %u\n  Size: %u  Flags: %u\n", sectionHeader->sh_type, sectionHeader->sh_addr, sectionHeader->sh_offset, sectionHeader->sh_size, sectionHeader->sh_flags);
    if(sectionHeader->sh_type == SHT_NOBITS) {
      void* s = KMalloc(sectionHeader->sh_size);
      Debug("Alocating memory at %u\n", s);
      Memset(s, 0, sectionHeader->sh_size);
      sectionHeader->sh_offset = (DWORD)s - (DWORD)header;
      // Debug("bits at %u\n", sectionHeader->sh_offset);
    //   Memcopy(programHeader->p_vaddr, data + programHeader->p_offset, programHeader->p_filesz);
    }
  }

  Debug("Program Headers: %u (%u bytes, offset %u)\n", header->e_phnum, header->e_phentsize, header->e_phoff);
  for(int i = 0; i < header->e_phnum; ++i) {
    ELFProgramHeader* programHeader = data + header->e_phoff + (i * header->e_phentsize);
    Debug("\tType: %u  Address: %u\n  Offset: %u\n  Size: %u %u\n", programHeader->p_type, programHeader->p_vaddr, programHeader->p_offset, programHeader->p_memsz, programHeader->p_filesz);
    if(programHeader->p_type == 1) {
      Memcopy(programHeader->p_vaddr, data + programHeader->p_offset, programHeader->p_filesz);
    }
  }

  CreateProcess(header->e_entry, processName, 1, commandLine);
  return S_OK; 
}

