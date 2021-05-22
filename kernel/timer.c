
#include <kernel.h>
#include <interrupt.h>
#include <console.h>
#include <process.h>

volatile int ticks = 0;
volatile int lastUptimeInSeconds = 0;
int ticksPerSecond = 100;
#define PIT_FREQUENCY 18.2067903

int TimerGetUptime() {
  return ticks / ticksPerSecond;
  // int uptime = (int)((float)ticks / PIT_FREQUENCY);
  // return uptime;
}

int TimerGetTicks() { return ticks; }

void TimerHandler(Registers* registers) {
  // if(TimerGetUptime() != lastUptimeInSeconds)
  if (ticks % 10 == 0) {
    lastUptimeInSeconds = TimerGetUptime();
    // Debug("Uptime: %ds %d", lastUptimeInSeconds, ticks);
    // Debug("%d\n", registers->eax);
    ProcessSchedule(registers);
  }
  ++ticks;
  return;
}

STATUS TimerSetFrequency(int hz) {
  int divisor = 1193180 / hz;            /* Calculate our divisor */
  IoWritePortByte(0x43, 0x36);           /* Set our command byte 0x36 */
  IoWritePortByte(0x40, divisor & 0xFF); /* Set low byte of divisor */
  IoWritePortByte(0x40, divisor >> 8);   /* Set high byte of divisor */
  return S_OK;
}

STATUS TimerInit(void) {
  TimerSetFrequency(ticksPerSecond);
  InstallIrqHandler(0, TimerHandler);
  return S_OK;
}

STATUS TimerDestroy(void) {
  RemoveIrqHandler(0);
  return S_OK;
}
