
#include <kernel.h>
#include <dma.h>

/*
* http://www.osdev.org/osfaq2/index.php/DMA
*/

STATUS DmaInit()
{
    int i;
    unsigned char* blah = (unsigned char*)FLOPPY_DMA_ADDRESS;
    Memset(blah, 0, FLOPPY_DMA_BUFFER_SIZE);

    /* Disable channel 2 */
    IoWritePortByte(0x0a, 6);

    /* Enable single write mode */
    /*IoWritePortByte(0x0b, 70);*/

    /* flip flop - can be any value*/
    IoWritePortByte(0xD8, 0xFF);

    /* offset address */
    IoWritePortByte(0x04, FLOPPY_DMA_ADDRESS & 0xFF);
    IoWritePortByte(0x04, (FLOPPY_DMA_ADDRESS & 0xFF00) >> 8);

    /* flip flop */
    IoWritePortByte(0xD8, 0xFF);


    /* count */
    IoWritePortByte(0x05, 0);
    IoWritePortByte(0x05, 0x24);

    /* page */
    IoWritePortByte(0x81, 0);

    /* Enable channel 2*/
    IoWritePortByte(0x0a, 0x02);
}


STATUS DmaEnableChannel(int channel)
{
    Assert(channel >= 0 && channel < MAX_DMA_CHANNELS);
    // fixme - enable channel
    return S_OK;
}


STATUS DmaDisableChannel(int channel)
{
    Assert(channel >= 0 && channel < MAX_DMA_CHANNELS);
    // fixme - disable channel
    return S_OK;
}


DmaFloppyRead()
{
    DmaInit();
    IoWritePortByte(0x0a, 0x06);
    IoWritePortByte(0x0b, 0x46);
    IoWritePortByte(0x0a, 0x02);
    /*
      out 0x0a, 0x06          ; mask DMA channel 2
        out 0x0b, 0x46          ; 01010110
                                ; single transfer, address increment, autoinit, read, channel2)
        out 0x0a, 0x02          ; unmask DMA channel 2
    	*/
}
