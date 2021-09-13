
#ifndef FLOPPY_H
#define FLOPPY_H

#define FLOPPY_IRQ 6

#define NO_FLOPPY 0
#define FLOPPY_360K_525 1
#define FLOPPY_12M_525 2
#define FLOPPY_720K_35 3
#define FLOPPY_144M_35 4
#define FLOPPY_288M_35 5

/* Initialize floppy driver */
STATUS FloppyInit(void);

/* Destroy floppy driver */
STATUS FloppyDestroy(void);

/* Enables the floppy drive's motor */
STATUS FloppyEnableMotor(void);

/* Disables the floppy drive's motor */
STATUS FloppyDisableMotor(void);

char* FloppyReadFile(char* name, int* size);

STATUS FloppyReadDirectory(char* name, struct _DirImpl* dirimpl);

#endif
