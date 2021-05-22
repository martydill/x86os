
/*
 * SerialPort.c
 * Serial port driver
 * References:
 *  http://wiki.osdev.org/Serial_ports
 */

#include <kernel.h>
#include <serialport.h>
#include <io.h>
#include <interrupt.h>
#include <device.h>
#include <string.h>

typedef struct ComPort_S {
  Device device;
} ComPort;

ComPort comPorts[NUM_COM_PORTS];

int SerialPortIsBusy() { return (IoReadPortByte(COM1_PORT + 5) & 0x20) == 0; }

void WriteCharacter(char a) {
  while (SerialPortIsBusy())
    ;
  IoWritePortByte(COM1_PORT, a);
}

void SerialPortWriteString(const char* format, ...) {
  va_list args;
  char buffer[2048];

  va_start(args, format);
  DoSprintf(sizeof(buffer), buffer, format, args);
  va_end(args);

  char* ptr = &buffer[0];
  while (*ptr) {
    WriteCharacter(*ptr);
    ++ptr;
  }

  return;
}

// Initialize serial port driver
STATUS SerialPortInit(void) {
  // Disable interrupts
  IoWritePortByte(COM1_PORT + REGISTER_INTERRUPT_ENABLE, 0);

  IoWritePortByte(COM1_PORT + REGISTER_LINE_CONTROL, 0x80); // set DLAB

  IoWritePortByte(COM1_PORT + REGISTER_DATA, 1); // baudrate LO

  IoWritePortByte(COM1_PORT + REGISTER_INTERRUPT_ENABLE,
                  0x01); // HI (0c => 9600 baud)

  IoWritePortByte(COM1_PORT + REGISTER_LINE_CONTROL,
                  0x03); // clear DLAB & set 8-N-1

  //    // Set to 8 data bits, 1.5/2 stop bits, no parity
  //    IoWritePortByte(COM1_PORT + REGISTER_LINE_CONTROL, (1 << 1));
  //
  //    // 1.5/2 stop bits
  //    IoWritePortByte(COM1_PORT + REGISTER_LINE_CONTROL, (1 << 1));
  //
  //    // 1.5/2 stop bits
  //    IoWritePortByte(COM1_PORT + REGISTER_LINE_CONTROL, (1 << 2));
  //

  IoWritePortByte(COM1_PORT + REGISTER_INTERRUPT_ID, 0xC7);

  IoWritePortByte(COM1_PORT + REGISTER_MODEM_CONTROL, 0x08);

  comPorts[0].device.Name = "Com1";
  DeviceRegister(&comPorts[0].device);

  //    Device kbDevice;
  //    kbDevice.Name = "COM1";
  //	kbDevice.Read = SerialRead;
  //	kbDevice.Status = 0;
  //	kbDevice.Status |= DEVICE_CAN_READ;
  //	kbDevice.Status |= DEVICE_OPEN;
  //
  //	DeviceRegister(&kbDevice);
  return S_OK;
}

// Destroy serial port driver
STATUS SerialPortDestroy(void) { return S_OK; }
