#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(void)
{
  static char buf[PR_MSG_PAGES * 4096 + 1];
  int n;

  n = dmesg(buf, sizeof(buf));
  if (n < 0) {
    fprintf(2, "dmesg: failed\n");
    exit(1);
  }
  write(1, buf, n);
  exit(0);
}
