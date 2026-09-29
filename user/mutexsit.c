#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
test_read_write(void)
{
  int fd, r;
  char buf[1];

  printf("-- read/write/fstat on a mutex --\n");

  fd = mutex();

  r = read(fd, buf, 1);
  if (r == -1)
    printf("OK: read returned -1\n");
  else
    printf("FAIL: read returned %d\n", r);

  r = write(fd, buf, 1);
  if (r == -1)
    printf("OK: write returned -1\n");
  else
    printf("FAIL: write returned %d\n", r);

  struct stat st;
  r = fstat(fd, &st);
  if (r == -1)
    printf("OK: fstat returned -1\n");
  else
    printf("FAIL: fstat returned %d\n", r);

  close(fd);
}

void
test_close_autounlock_self(void)
{
  int fd, fd2, r;

  printf("-- close() by the holder auto-unlocks --\n");

  fd = mutex();
  mutex_lock(fd);
  fd2 = dup(fd);
  close(fd);

  r = mutex_lock(fd2);
  if (r == 0)
    printf("OK: mutex was unlocked by close()\n");
  else
    printf("FAIL: mutex_lock after close returned %d\n", r);

  mutex_unlock(fd2);
  close(fd2);
}

void
test_close_by_other_no_release(void)
{
  int fd, holder_pid, waiter_pid;

  printf("-- close() by a non-holder must not release the lock --\n");

  fd = mutex();

  holder_pid = fork();
  if (holder_pid == 0) {
    mutex_lock(fd);
    printf("holder: locked\n");
    pause(30);
    printf("holder: unlocking\n");
    mutex_unlock(fd);
    exit(0);
  }

  pause(5);

  waiter_pid = fork();
  if (waiter_pid == 0) {
    printf("waiter: trying to lock\n");
    mutex_lock(fd);
    printf("waiter: got the lock\n");
    mutex_unlock(fd);
    exit(0);
  }

  pause(2);
  close(fd);

  wait(0);
  wait(0);
  printf("OK: waiter had to wait for holder to unlock (see order above)\n");
}

void
test_unlock_by_other(void)
{
  int fd, pid, r;

  printf("-- mutex_unlock() by a non-holder --\n");

  fd = mutex();

  pid = fork();
  if (pid == 0) {
    mutex_lock(fd);
    pause(20);
    mutex_unlock(fd);
    exit(0);
  }

  pause(5);
  r = mutex_unlock(fd);
  if (r == -1)
    printf("OK: mutex_unlock by non-owner returned -1\n");
  else
    printf("FAIL: mutex_unlock by non-owner returned %d\n", r);

  wait(0);
  close(fd);
}

void
test_unlock_unlocked(void)
{
  int fd, r;

  printf("-- mutex_unlock() of an unlocked mutex --\n");

  fd = mutex();
  r = mutex_unlock(fd);
  if (r == -1)
    printf("OK: mutex_unlock of an unlocked mutex returned -1\n");
  else
    printf("FAIL: mutex_unlock of an unlocked mutex returned %d\n", r);

  close(fd);
}

void
test_exit_releases(void)
{
  int fd, pid, r;

  printf("-- exit() while holding a mutex --\n");

  fd = mutex();

  pid = fork();
  if (pid == 0) {
    mutex_lock(fd);
    exit(0);
  }
  wait(0);

  r = mutex_lock(fd);
  if (r == 0)
    printf("OK: exit() released the mutex the child was holding\n");
  else
    printf("FAIL: mutex_lock after child exit returned %d\n", r);

  mutex_unlock(fd);
  close(fd);
}

void
test_exit_wakes_waiter(void)
{
  int fd, holder_pid, waiter_pid;

  printf("-- killing the holder must wake a blocked waiter --\n");

  fd = mutex();

  holder_pid = fork();
  if (holder_pid == 0) {
    mutex_lock(fd);
    pause(1000);
    exit(0);
  }

  pause(5);

  waiter_pid = fork();
  if (waiter_pid == 0) {
    printf("waiter: trying to lock a mutex held by a process about to be killed\n");
    mutex_lock(fd);
    printf("waiter: got the lock\n");
    mutex_unlock(fd);
    exit(0);
  }

  pause(10);
  kill(holder_pid);
  wait(0);
  wait(0);
  printf("OK: waiter was woken up once the holder was killed\n");

  close(fd);
}

void
test_kill_waiter(void)
{
  int fd, waiter_pid;

  printf("-- killing a process blocked in mutex_lock() --\n");

  fd = mutex();
  mutex_lock(fd);

  waiter_pid = fork();
  if (waiter_pid == 0) {
    mutex_lock(fd);
    exit(0);
  }

  pause(10);
  kill(waiter_pid);
  wait(0);
  printf("OK: killed waiter was reaped, did not hang\n");

  mutex_unlock(fd);
  close(fd);
}

int
main(void)
{
  test_read_write();
  test_close_autounlock_self();
  test_close_by_other_no_release();
  test_unlock_by_other();
  test_unlock_unlocked();
  test_exit_releases();
  test_exit_wakes_waiter();
  test_kill_waiter();

  printf("ALL DONE\n");
  exit(0);
}
