
#ifndef SERIALPORT_H
#define SERIALPORT_H

#define NUM_COM_PORTS   4

#define SERIAL_IRQ  4

#define COM1_PORT 	0x3F8
#define COM2_PORT 	0x2F8
#define COM3_PORT 	0x3E8
#define COM4_PORT 	0x2E8


#define REGISTER_DATA               0
#define REGISTER_INTERRUPT_ENABLE   1
#define REGISTER_DIVISOR_LSB        0
#define REGISTER_DIVISOR_MSB        1
#define REGISTER_INTERRUPT_ID       2
#define REGISTER_LINE_CONTROL       3
#define REGISTER_MODEM_CONTROL      4
#define REGISTER_LINE_STATUS        5
#define REGISTER_SCRATCH            6


/* Initialize SerialPort driver */
STATUS SerialPortInit(void);

/* Destroy SerialPort driver */
STATUS SerialPortDestroy(void);



#endif
