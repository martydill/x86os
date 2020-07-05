#include <kernel.h>
#include <elf.h>


STATUS ELFParseFile(BYTE* data)
{
  if(data == NULL) 
  {
    return S_FAIL;
  }

  ELFHeader* header = (ELFHeader*)data;
  KPrint("Ident: %s\n", header->e_ident);
  KPrint("Type: %d\n", header->e_type);
  KPrint("Entry: %u\n", header->e_entry);

  void* addr = 1024 * 1024 * 8;

  KPrint("Program Headers: %u (%u bytes, offset %u)\n", header->e_phnum, header->e_phentsize, header->e_phoff);
  for(int i = 0; i < header->e_phnum; ++i) {
    ELFProgramHeader* programHeader = data + header->e_phoff + (i * header->e_phentsize);
    KPrint("\tType: %u  Address: %u\n  Offset: %u\n  Size: %u\n", programHeader->p_type, programHeader->p_vaddr, programHeader->p_offset, programHeader->p_memsz);
    if(programHeader->p_type == 1) {
      Memcopy(programHeader->p_vaddr, data + programHeader->p_offset, programHeader->p_filesz);
    }
  }
  CreateProcess(addr, "test", 1);
  return S_OK; 
}

