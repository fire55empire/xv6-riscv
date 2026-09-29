#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
classbit(char *name)
{
  if (strcmp(name, "syscall") == 0)
    return TRACE_SYSCALL;
  if (strcmp(name, "intr") == 0)
    return TRACE_INTR;
  if (strcmp(name, "proc") == 0)
    return TRACE_PROC;
  if (strcmp(name, "exec") == 0)
    return TRACE_EXEC;
  if (strcmp(name, "all") == 0)
    return TRACE_ALL;
  return 0;
}

int
main(int argc, char *argv[])
{
  int mask = 0;
  int duration = 0;

  if (argc == 2 && strcmp(argv[1], "off") == 0) {
    if (trace(0, 0) < 0)
      exit(1);
    exit(0);
  }

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] >= '0' && argv[i][0] <= '9')
      duration = atoi(argv[i]);
    else
      mask |= classbit(argv[i]);
  }

  if (mask == 0) {
    fprintf(2, "usage: trace off | trace syscall|intr|proc|exec|all... [ticks]\n");
    exit(1);
  }
  if (trace(mask, duration) < 0) {
    fprintf(2, "trace: failed\n");
    exit(1);
  }
  exit(0);
}
