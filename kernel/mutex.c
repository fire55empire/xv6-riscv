#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

int
mutexalloc(struct file **f)
{
  struct file *newfile;
  struct sleeplock *lk;

  newfile = filealloc();
  if (newfile == 0) {
    *f = 0;
    return -1;
  }

  lk = (struct sleeplock *)kalloc();
  if (lk == 0) {
    fileclose(newfile);
    *f = 0;
    return -1;
  }

  initsleeplock(lk, "mutex");

  newfile->type = FD_MUTEX;
  newfile->readable = 0;
  newfile->writable = 0;
  newfile->mlock = lk;

  *f = newfile;
  printk("mutexalloc: pid %d lock %p\n", myproc()->pid, lk);
  return 0;
}

void
mutexclose(struct sleeplock *lk)
{
  printk("mutexclose: pid %d lock %p\n", myproc()->pid, lk);
  kfree((char *)lk);
}

int
mutexlock(struct sleeplock *lk)
{
  struct proc *p = myproc();

  acquire(&lk->lk);
  while (lk->locked == 1) {
    if (killed(p)) {
      release(&lk->lk);
      return -1;
    }
    sleep_prepare(lk);
    release(&lk->lk);
    sleep();
    acquire(&lk->lk);
  }

  lk->locked = 1;
  lk->pid = p->pid;
  release(&lk->lk);
  return 0;
}

int
mutexunlock(struct sleeplock *lk)
{
  if (!holdingsleep(lk))
    return -1;

  releasesleep(lk);
  return 0;
}
