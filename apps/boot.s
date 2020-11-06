extern _start2

global _start

align 4

_start:
  xor ebp,ebp
  push ebx ; argv
  push eax ; argc
  call _start2
