
#ifndef KEYBOARD_H
#define KEYBOARD_H

/* IRQ of the keyboard */
#define KB_IRQ 1

/* Keyboard data register */
#define KB_DATA_REGISTER 0x60

/* Keyboard control register */
#define KB_CONTROL_REGISTER 0x64

STATUS KbInit(void);
STATUS KbDestroy(void);

#endif
