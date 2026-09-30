#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int failed = 0;

static void
check(int cond, char *msg)
{
  if (cond) {
    printf("ok: %s\n", msg);
  } else {
    printf("FAIL: %s\n", msg);
    failed = 1;
  }
}

static void
test_count(void)
{
  int n = ps_listinfo(0, 0);
  check(n >= 1, "count query returns at least the caller");
}

static void
test_toobig(void)
{
  struct procinfo buf[1];
  int r = ps_listinfo(buf, 0);
  check(r == PS_ERR_TOOBIG, "lim=0 reports PS_ERR_TOOBIG");
}

static void
test_badaddr(void)
{
  int r = ps_listinfo((struct procinfo *)0x10000000, 100);
  check(r == PS_ERR_FAULT, "out-of-range address reports PS_ERR_FAULT");
  check(PS_ERR_FAULT != PS_ERR_TOOBIG,
        "PS_ERR_FAULT is distinct from PS_ERR_TOOBIG");
}

static void
test_self(void)
{
  struct procinfo buf[64];
  int n, i, found;

  n = ps_listinfo(buf, 64);
  check(n > 0, "listing with a big-enough buffer succeeds");

  found = 0;
  for (i = 0; i < n; i++) {
    if (buf[i].pid == getpid()) {
      found = 1;
      check(strcmp(buf[i].name, "pstest") == 0,
            "own entry has the right name");
      if (buf[i].state == 4) {
        check(1, "own entry is RUNNING");
      } else {
        check(0, "own entry is RUNNING");
      }
    }
  }
  check(found, "own pid appears in the listing");
}

static void
test_child_ppid(void)
{
  struct procinfo buf[64];
  int n, i, found, pid, xstatus;

  pid = fork();
  if (pid < 0) {
    check(0, "fork for child_ppid test");
    return;
  }
  if (pid == 0) {
    pause(100);
    exit(0);
  }

  pause(2);
  n = ps_listinfo(buf, 64);
  found = 0;
  for (i = 0; i < n; i++) {
    if (buf[i].pid == pid) {
      found = 1;
      check(buf[i].ppid == getpid(), "child's ppid matches the parent");
    }
  }
  check(found, "child appears in the listing while alive");

  kill(pid);
  wait(&xstatus);
}

int
main(void)
{
  test_count();
  test_toobig();
  test_badaddr();
  test_self();
  test_child_ppid();

  if (failed) {
    printf("SOME TESTS FAILED\n");
    exit(1);
  }
  printf("ALL TESTS PASSED\n");
  exit(0);
}
