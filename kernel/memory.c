
/*
* RtlMem.c
* Run-time library memory handling routines
*/

#include <kernel.h>

/* Sets a region of memory to the specified value */
STATUS Memset(BYTE* string, BYTE value, int size)
{
    int i = 0;

    /*Assert(string != NULL);

    if(string == NULL)
    	return S_FAIL;*/

    /* FIXME: Optimize this */
    for(i = 0; i < size; ++i)
    {
        string[i] = value;
    }

    return S_OK;
}


/* Copies a region of memory to another region */
STATUS Memcopy(BYTE* dest, const BYTE* src, int size)
{
    int i;

    if(dest == NULL || src == NULL)
        return S_FAIL;

    /* FIXME: Optimize this */
    for(i = 0; i < size; ++i)
    {
        *dest++ = *src++;
    }

    return S_OK;
}


void Test_Memset(void)
{
    return;
}


void Test_Memcopy(void)
{
    return;
}


void Test_Memory(void)
{
    /* fixme: test these */
    Test_Memset();
    Test_Memcopy();
}
