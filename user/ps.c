#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

static void
printfield(char *s, int width)
{
  int len;

  printf("%s", s);
  for (len = strlen(s); len < width; len++)
    printf(" ");
}

static void
itoa(int v, char *buf)
{
  char tmp[12];
  int i, j, neg, n;

  if (v < 0)
    neg = 1;
  else
    neg = 0;

  if (neg == 1)
    n = -v;
  else
    n = v;

  i = 0;
  if (n == 0) {
    tmp[i] = '0';
    i = i + 1;
  }
  while (n > 0) {
    tmp[i] = '0' + (n % 10);
    i++;
    n = n / 10;
  }
  if (neg == 1) {
    tmp[i] = '-';
    i++;
  }

  j = 0;
  while (i > 0) {
    i--;
    buf[j] = tmp[i];
    j++;
  }
  buf[j] = 0;
}

static char *
statename(int state)
{
  static char *names[] = {"UNUSED",   "USED",    "SLEEPING",
                           "RUNNABLE", "RUNNING", "ZOMBIE"};
  if (state < 0 || state >= (int)(sizeof(names) / sizeof(names[0])))
    return "?";
  return names[state];
}

static char *
findname(struct procinfo *buf, int n, int pid)
{
  int i;

  if (pid == 0)
    return "-";
  for (i = 0; i < n; i++)
    if (buf[i].pid == pid)
      return buf[i].name;
  return "-";
}

int
main(void)
{
  struct procinfo *buf;
  int cap, n, i;
  char numbuf[12];

  cap = 16;
  while (1) {
    buf = malloc(cap * sizeof(struct procinfo));
    if (buf == 0) {
      fprintf(2, "ps: out of memory\n");
      exit(1);
    }
    n = ps_listinfo(buf, cap);
    if (n == PS_ERR_TOOBIG) {
      free(buf);
      cap = cap * 2;
    } else {
      break;
    }
  }

  if (n < 0) {
    fprintf(2, "ps: ps_listinfo failed: %d\n", n);
    exit(1);
  }

  printfield("PID", 6);
  printfield("NAME", 16);
  printfield("STATE", 11);
  printfield("PPID", 6);
  printf("PNAME\n");

  for (i = 0; i < n; i++) {
    itoa(buf[i].pid, numbuf);
    printfield(numbuf, 6);
    printfield(buf[i].name, 16);
    printfield(statename(buf[i].state), 11);
    itoa(buf[i].ppid, numbuf);
    printfield(numbuf, 6);
    printf("%s\n", findname(buf, n, buf[i].ppid));
  }

  free(buf);
  exit(0);
}
