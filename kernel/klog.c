#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

#define KLOG_SIZE (PR_MSG_PAGES * PGSIZE)

static struct {
  struct spinlock lock;
  char buf[KLOG_SIZE];
  uint64 nwritten;
  int trace_mask;
  uint trace_until;
} klog;

static struct sleeplock snaplock;
static char snap[KLOG_SIZE + 1];

void
kloginit(void)
{
  initlock(&klog.lock, "klog");
  initsleeplock(&snaplock, "klogsnap");
  klog.buf[0] = '\n';
  klog.nwritten = 1;
}

static void
klog_putc(int c)
{
  klog.buf[klog.nwritten % KLOG_SIZE] = c;
  klog.nwritten++;
}

static uint
gettime(void)
{
  uint t;

  acquire(&tickslock);
  t = ticks;
  release(&tickslock);
  return t;
}

void
pr_msg(const char *fmt, ...)
{
  va_list ap;
  char digits[12];
  uint t = gettime();
  int n = 0;

  do {
    digits[n++] = '0' + t % 10;
    t /= 10;
  } while (t > 0);

  acquire(&klog.lock);
  klog_putc('[');
  while (n > 0)
    klog_putc(digits[--n]);
  klog_putc(']');
  klog_putc(' ');
  va_start(ap, fmt);
  vformat(klog_putc, fmt, ap);
  va_end(ap);
  klog_putc('\n');
  release(&klog.lock);
}

int
klog_set_trace(int mask, int duration)
{
  uint now = gettime();

  if ((mask & ~TRACE_ALL) != 0 || duration < 0)
    return -1;

  acquire(&klog.lock);
  klog.trace_mask = mask;
  if (duration > 0)
    klog.trace_until = now + duration;
  else
    klog.trace_until = 0;
  release(&klog.lock);
  return 0;
}

int
klog_traced(int bit)
{
  int mask;
  uint until;

  acquire(&klog.lock);
  mask = klog.trace_mask;
  until = klog.trace_until;
  release(&klog.lock);

  if ((mask & bit) == 0)
    return 0;
  if (until != 0 && gettime() >= until)
    return 0;
  return 1;
}

int
klog_copyout(uint64 dst, int n)
{
  struct proc *p = myproc();
  uint64 first, len, i;
  int ret;

  if (n <= 0)
    return -1;

  acquiresleep(&snaplock);

  acquire(&klog.lock);
  len = klog.nwritten;
  first = 0;
  if (len > KLOG_SIZE) {
    len = KLOG_SIZE;
    first = klog.nwritten - KLOG_SIZE;
  }
  while (len > 0 && klog.buf[first % KLOG_SIZE] != '\n') {
    first++;
    len--;
  }
  if (len > 0) {
    first++;
    len--;
  }
  if (len > n - 1)
    len = n - 1;
  for (i = 0; i < len; i++)
    snap[i] = klog.buf[(first + i) % KLOG_SIZE];
  snap[len] = 0;
  release(&klog.lock);

  ret = len;
  if (copyout(p->pagetable, p->sz, dst, snap, len + 1) < 0)
    ret = -1;

  releasesleep(&snaplock);
  return ret;
}
