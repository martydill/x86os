#include <kernel.h>
#include <elf.h>

int count = 0;
ProcessId ELFParseFile(BYTE* data, char* processName, char* commandLine,
                       BYTE priority) {
  if (data == NULL) {
    return S_FAIL;
  }

  ELFHeader* header = (ELFHeader*)data;
  Debug("Running %s\n", processName);
  Debug("Ident: %s\n", header->e_ident);
  Debug("Type: %d\n", header->e_type);
  Debug("Entry: %u\n", header->e_entry);

  void* addr =
      (void*)(1024 * 1024 * 64 + count * 1024 * 1024 * 4); // 1024 * 1024 * 8;
  Debug("For physical address %u and data located at %c %c %c %c\n", addr,
        data[0], data[1], data[2], data[3]);
  count++;
  Debug("Section Headers: %u (%u bytes, offset %u)\n", header->e_shnum,
        header->e_shentsize, header->e_shoff);
  for (int i = 0; i < header->e_shnum; ++i) {
    ELFSectionHeader* sectionHeader =
        (ELFSectionHeader*)(data + header->e_shoff + (i * header->e_shentsize));
    Debug("\tType: %u  Address: %u\n  Offset: %u\n  Size: %u  Flags: %u\n",
          sectionHeader->sh_type, sectionHeader->sh_addr,
          sectionHeader->sh_offset, sectionHeader->sh_size,
          sectionHeader->sh_flags);
    if (sectionHeader->sh_type == SHT_NOBITS) {
      void* s = (void*)KMalloc(sectionHeader->sh_size);
      Debug("Alocating memory at %u\n", s);
      Memset(s, 0, sectionHeader->sh_size);
      sectionHeader->sh_offset = (DWORD)s - (DWORD)header;
      // Debug("bits at %u\n", sectionHeader->sh_offset);
      //   Memcopy(programHeader->p_vaddr, data + programHeader->p_offset,
      //   programHeader->p_filesz);
    }
  }

  Debug("Program Headers: %u (%u bytes, offset %u)\n", header->e_phnum,
        header->e_phentsize, header->e_phoff);
  for (int i = 0; i < header->e_phnum; ++i) {
    ELFProgramHeader* programHeader =
        (ELFProgramHeader*)(data + header->e_phoff + (i * header->e_phentsize));
    Debug("\tType: %u  Address: %u\n  Offset: %u\n  Size: %u %u\n",
          programHeader->p_type, programHeader->p_vaddr,
          programHeader->p_offset, programHeader->p_memsz,
          programHeader->p_filesz);
    if (programHeader->p_type == 1) {
      Debug("Loading %s at %u\n", commandLine, addr);
      Memcopy(addr, data + programHeader->p_offset, programHeader->p_filesz);
    }
  }

  Debug("Creating new process for entry %u with command line '%s'\n",
        header->e_entry, commandLine);
  return CreateProcess(header->e_entry, processName, priority, commandLine);
}
