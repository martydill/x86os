
#ifndef TSS_H
#define TSS_H

#include <gdt.h>

// Based on https://www.sandpile.org/x86/tss.htm
struct TSS_S {
  DWORD Link;
  DWORD ESP0;
  DWORD SS0;
  DWORD ESP1;
  DWORD SS1;
  DWORD ESP2;
  DWORD SS2;
  DWORD CR3;
  DWORD EIP;
  DWORD EFLAGS;
  DWORD EAX;
  DWORD ECX;
  DWORD EDX;
  DWORD EBX;
  DWORD ESP;
  DWORD EBP;
  DWORD ESI;
  DWORD EDI;
  DWORD ES;
  DWORD CS;
  DWORD SS;
  DWORD DS;
  DWORD FS;
  DWORD GS;
  DWORD LDTR;
  DWORD IOPB;
} __attribute__((packed));

typedef struct TSS_S TSS;

STATUS LoadTSS(GDTEntry* gdt);

#endif
