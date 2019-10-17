
/*
* Floppy.c
* Floppy disk driver
* References:
*  http://www.isdaman.com/alsos/hardware/fdc/floppy.htm
*/


#include <kernel.h>
#include <floppy.h>
#include <io.h>
#include <interrupt.h>
#include <device.h>

const char* floppyTypeString[] =
{
    "No floppy drive",
    "360k 5.25\"",
    "1.2m 5.25\"",
    "720k 3.5\"",
    "1.44m 3.5\"",
    "2.88m 3.5\""
};


enum FloppyCommands
{
    READ_SECTOR = 0xe6,
    SEEK_TRACK = 0x0f,
    SENSE_INTERRUPT = 0x08, /* or 'check interrupt status */
    SPECIFY = 0x03,
    RECALIBRATE = 0x07
};

enum FloppyRegisters
{
    BASE		= 0x3f0,
    STATUSA		= 0x3f0,
    STATUSB		= 0x3f1,
    DOR			= 0x3f2,
    MSR			= 0x3f4,
    DATARATE	= 0x3f4,
    DATA		= 0x3f5,
    DIR			= 0x3f7,
    CCR			= 0x3f7
};

enum DORFlags
{
    DR0  = 1 << 0,
    DR1  = 1 << 1,
    REST = 1 << 2,
    DMA  = 1 << 3,
    MOTA = 1 << 4,
    MOTB = 1 << 5,
    MOTC = 1 << 6,
    MOTD = 1 << 7
};

enum MSRFlags
{
    ACTA = 1 << 0,
    ACTB = 1 << 1,
    ACTC = 1 << 2,
    ACTD = 1 << 3,
    BUSY = 1 << 4,
    NDMA = 1 << 5,
    DIO  = 1 << 6,
    MRQ  = 1 << 7
};

enum DIRFlags
{
    RAT0 = 1 << 1,
    RAT1 = 1 << 2,
    CHAN = 1 << 7
};

enum DataTransferRate
{
    RATE_500kbit,
    RATE_300kbit,
    RATE_250kbit,
    RATE_1mbit
};

typedef struct FloppyDrive_S
{
    BYTE dor;
    BYTE msr;
    BYTE present;
    int track;
    int HeadsPerCylinder;
    int SectorsPerTrack;
    Device device;
} FloppyDrive;


FloppyDrive drives[2];
volatile int intcount = 0;
void GetDriveStatus(FloppyDrive* drive);
void sleep(void)
{
    int ticks = TimerGetTicks();
    while(TimerGetTicks() - ticks < 10) { }
}

/* Waits for the MRQ bit of the MSR to be set */
void WaitForReady(FloppyDrive* drive)
{
    while(1)
    {
        GetDriveStatus(drive);

        if(drive->msr & MRQ)
            break;
    }
}


void WriteByte(BYTE byte)
{
    volatile unsigned msr;
    unsigned          tmo;

    WaitForReady(&drives[0]);
    for (tmo = 0; tmo < 128; tmo++)
    {
        msr = IoReadPortByte(MSR);
        if ((msr & 0xC0) == 0x80)
        {
            IoWritePortByte(DATA, byte);
            return;
        }
        IoReadPortByte(0x80); /* delay */
    }
    return; /* write timeout */
}


BYTE ReadByte()
{
    volatile unsigned msr;
    unsigned short timeout;

    for (timeout = 0; timeout < 128; timeout++)
    {
        msr = IoReadPortByte(MSR);
        if ((msr & 0xD0) == 0xD0) return IoReadPortByte(DATA);
        IoReadPortByte(0x80); /* delay */
    }

    return 0; /* read timeout */
}


volatile int expectingInterrupt = 0;

/* IRQ 6 handler */
void FloppyHandler(Registers* registers)
{
    expectingInterrupt = 1;
}


/* Starts the motor and waits a while for it to turn on */
void StartMotor(FloppyDrive* drive)
{
    Assert(drive != NULL);
    drive->dor |= MOTA;
    IoWritePortByte(DOR, drive->dor);
    sleep();
}
void WaitForInterrupt();
void DiskRecalibrate()
{
    WriteByte(RECALIBRATE);
    WriteByte(0);

    WaitForInterrupt();

    WriteByte(SENSE_INTERRUPT);
    ReadByte();
    ReadByte();
}

void DiskReset(FloppyDrive* drive)
{
    Assert(drive != NULL);

    drive->dor = 0;
    IoWritePortByte(DOR, drive->dor);

    drive->dor |= DMA;
    drive->dor |= REST;
    drive->dor |= MOTA;
    IoWritePortByte(DOR, drive->dor);

    WaitForInterrupt();
    int i = 0;
    for(i = 0; i < 4; ++i)
    {
        WriteByte(SENSE_INTERRUPT);
        ReadByte();
        ReadByte();
    }

    IoWritePortByte(CCR, 0);
    WriteByte(SPECIFY);
    WriteByte(0xdf);
    WriteByte(0x02);
}

void GetDriveStatus(FloppyDrive* drive)
{
    BYTE b = IoReadPortByte(MSR);
    drive->msr = b;
}


BOOL DiskChanged()
{
    BYTE b = IoReadPortByte(DIR);
    if(b & CHAN)
        return TRUE;

    return FALSE;
}


void WaitForReady2()
{
    int c = 0;

    while ((c < 7) && (IoReadPortByte(MSR) & 0x10))
    {
        ReadByte();
        c++;
    }
}


/* busy waits until we receive an interrupt */
void WaitForInterrupt()
{
    while(!expectingInterrupt) {}
    expectingInterrupt = 0;
}


/* Seeks to the given track */
STATUS FloppySeek(unsigned int track)
{
    if(drives[0].track == track)
    {
        /*KPrint("Already on track %u\n", track);*/
        return S_OK;
    }

    /* fixme - start motor if it's not already started */
    KPrint("Seeking to track %u ", track);
    WriteByte(SEEK_TRACK);
    WriteByte(0);
    WriteByte(track);
    WaitForInterrupt();
    sleep();

    /* Ensure that the seek was successful */
    WriteByte(SENSE_INTERRUPT);
    BYTE b = ReadByte();
    BYTE b2 = ReadByte();

    KPrint("Now on track %d, sr0=%d\n", (int)b2, (int)b);
    if(b2 != track)
    {
        /* fixme: reset? */
        KPrint("Seek failed!\n");
        return S_FAIL;
    }

    drives[0].track = track;
    return S_OK;
}

int dir = 0;

/* Reads the given sector into the supplied buffer */
STATUS FloppyReadSector(int sector, char* buffer)
{
    Assert(buffer != NULL);
    int LBA = sector;

    int HPC = 2;
    int SPT = 18;
    int CYL = LBA / (HPC * SPT);
    int TEMP = LBA % (HPC * SPT);
    int HEAD = TEMP / SPT;
    int SECT = TEMP % SPT + 1;

    /*	KPrint("LBA: %d  CYL: %d  HEAD: %d  SECT: %d\n", LBA, CYL, HEAD, SECT);*/

    if(DiskChanged())
    {
        KPrint("Disk changed\n");
        return S_FAIL;
    }

    FloppySeek(CYL);

    IoWritePortByte(CCR, 0);

    DmaFloppyRead();
    WriteByte(READ_SECTOR);
    WriteByte((HEAD << 2));
    WriteByte(CYL);
    WriteByte(HEAD);
    WriteByte(SECT);
    WriteByte(2);
    WriteByte(18);
    WriteByte(0x1b);
    WriteByte(0xff);

    WaitForReady(&drives[0]);
    WaitForInterrupt();

    /* clear up the fifo queue */
    int z = 0;
    for(z = 0; z < 7; ++z)
        ReadByte();

    int i;
    unsigned char* blah = (unsigned char*)0x0000;
    for(i = 0; i < 512; ++i)
    {
        buffer[i] = blah[i];
    }

    if(dir == 0)
        dir = FATParseBootSector(buffer);

    /* fixme: check for errors */
    /* fixme: turn off motor */
    /* fixme: use memcopy */
    return S_OK;
}


void read(int z)
{
    int i;
    char buf[512];
    FloppyReadSector(z, buf);
    /*for(i = 0; i < 70; ++i)
    	KPrint("%c", buf[i]);*/

    if(dir != 0)
    {
        FloppyReadSector(dir, buf);
        FATReadDirectory(buf);
        /*for(i = 0; i < 70; ++i)
        KPrint("%c", buf[i]);*/
    }
}


/* Initialize floppy driver */
STATUS FloppyInit(void)
{
    BYTE floppyInfo, driveA, driveB;
    IoWritePortByte(0x70, 0x10);
    floppyInfo = IoReadPortByte(0x71);

    driveA = floppyInfo >> 4;
    driveB = floppyInfo & 0xF;

    drives[0].present = driveA != NO_FLOPPY;
    drives[1].present = driveB != NO_FLOPPY;

    KPrint("Floppy A: id %d, type %s\n", driveA, floppyTypeString[driveA]);
    KPrint("Floppy B: id %d, type %s\n", driveB, floppyTypeString[driveB]);

    InstallIrqHandler(FLOPPY_IRQ, FloppyHandler);

    int i;
    for(i = 0; i < 2; ++i)
    {
        if(drives[i].present)
        {
            DiskReset(&drives[i]);

            GetDriveStatus(&drives[i]);

            /*if(drives[i].msr & MRQ)
            	KPrint("Ready - ");
            else
            	KPrint("Not ready - ");

            if(drives[i].msr & DIO)
            	KPrint("controller to cpu - ");
            else
            	KPrint("cpu to controller - ");

            if(drives[i].msr & NDMA)
            	KPrint("no DMA - ");
            else
            	KPrint("DMA - ");

            if(drives[i].msr & BUSY)
            	KPrint("busy");
            else
            	KPrint("not busy");

            KPrint("\n");*/

            drives[i].device.Name = KMalloc(16);
            Sprintf(16, drives[i].device.Name, "Floppy%d", i);
            drives[i].device.Status = DEVICE_OPEN;
            DeviceRegister(&drives[i].device);
        }
    }
    return S_OK;
}


/* Destroy floppy driver */
STATUS FloppyDestroy(void)
{
    RemoveIrqHandler(FLOPPY_IRQ);
    return S_OK;
}
