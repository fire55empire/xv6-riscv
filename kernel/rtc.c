#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

#define ReadReg(a) (*(volatile uint32 *)(a))

static struct spinlock rtclock;

void
rtcinit(void)
{
  initlock(&rtclock, "rtc");
}

uint32
rtc_read_low(void)
{
  return ReadReg(RTC_TIME_LOW);
}

uint32
rtc_read_high(void)
{
  return ReadReg(RTC_TIME_HIGH);
}

uint64
rtc_read(void)
{
  uint64 lo, hi;

  acquire(&rtclock);
  lo = rtc_read_low();
  hi = rtc_read_high();
  release(&rtclock);
  return (hi << 32) + lo;
}
