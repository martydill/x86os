
/*
* Debug.c
* Contains a few routines used for debugging
*/


/* Halts execution */
void DbgPanic(void)
{
    while(1)
    {
        __asm__("hlt");
    }
}
