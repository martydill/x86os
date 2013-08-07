
// from http://wiki.osdev.org/FPU

#include <kernel.h>

int FPU_CW;

void set_fpu_control_word(const int cw)
{
    // if(cpuid_features.FPU) // checks for the FPU flag
    //{
    // FSTCW = Store FPU Control Word
    // FLDCW = Load FPU Control Word
    __asm__ __volatile__("fldcw (%0);    "    // sets the FPU control word to "cw"
                         "fstcw (%1);    "    // store the FPU control word into FPU_CW
                         ::"r"(cw), "r"(&FPU_CW));
    //}
}

void setup_x87_fpu()
{
    DWORD cr4; // backup of CR4

//   if(cpuid_features.FPU) // checks for the FPU flag
//  {
    // place CR4 into our variable
    __asm__ __volatile__("mov %%cr4, %0;" : "=r" (cr4));

    // set the OSFXSR bit
    cr4 |= 0x200;

    // reload CR4 and INIT the FPU (FINIT)
    __asm__ __volatile__("mov %0, %%cr4; finit;" : : "r"(cr4));

    // set the FPU Control Word
    set_fpu_control_word(0xB7F);
    // }
}
