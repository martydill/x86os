
/*
* startup.c
* Contains low-level startup function which passes the boot info to the kernel
*/

#include <boot.h>

extern int KeMain(MultibootInfo* bootInfo);


int _start(void* param)
{
    KeMain(param);
    return 0;
}
