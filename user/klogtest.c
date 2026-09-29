#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static char buf[PR_MSG_PAGES * 4096 + 1];

void
check(int ok, char *what)
{
  if (!ok) {
    printf("klogtest: FAIL: %s\n", what);
    exit(1);
  }
}

int
count_lines(char *s)
{
  int lines = 0;

  while (*s) {
    check(*s == '[', "line starts with '['");
    while (*s && *s != '\n')
      s++;
    check(*s == '\n', "line ends with newline");
    s++;
    lines++;
  }
  return lines;
}

int
main(void)
{
  int n, i;
  int fds[2];
  char c;

  check(trace(TRACE_ALL, 0) == 0, "trace on");
  pipe(fds);
  for (i = 0; i < 4; i++) {
    if (fork() == 0) {
      write(fds[1], "x", 1);
      pause(1);
      exit(i);
    }
  }
  for (i = 0; i < 4; i++) {
    read(fds[0], &c, 1);
    wait(0);
  }
  trace(0, 0);

  n = dmesg(buf, sizeof(buf));
  check(n > 0 && n == strlen(buf), "dmesg length");
  printf("klogtest: %d lines, %d bytes\n", count_lines(buf), n);

  check(trace(1 << 20, 0) < 0, "bad mask rejected");

  check(trace(TRACE_SYSCALL, 2) == 0, "timed trace");
  pause(5);
  n = dmesg(buf, sizeof(buf));
  check(n > 0, "dmesg after timed trace");
  trace(0, 0);

  trace(TRACE_SYSCALL, 0);
  for (i = 0; i < 3000; i++)
    getpid();
  trace(0, 0);
  n = dmesg(buf, sizeof(buf));
  check(n > 0 && n < sizeof(buf), "wrapped dmesg length");
  count_lines(buf);

  n = dmesg(buf, 20);
  check(n == 19 && buf[19] == 0, "short buffer");

  printf("klogtest: ok\n");
  exit(0);
}
