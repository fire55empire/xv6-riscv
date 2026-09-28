#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SIZE 64
#define MAX_DIGITS 9

static int
read_line(char *buf, int size)
{
  int len = 0;
  int overflow = 0;
  char c;

  for (;;) {
    int r = read(0, &c, 1);
    if (r < 0)
      return -1;
    if (r == 0 || c == '\n')
      break;
    if (len + 1 >= size)
      overflow = 1;
    else
      buf[len++] = c;
  }
  buf[len] = '\0';
  return overflow ? -2 : 0;
}

static int
my_atoi(char *ptr)
{
  int ans = 0;
  int is_neg = 0;

  if (*ptr == '-') {
    ptr++;
    is_neg = 1;
  }
  while (*ptr >= '0' && *ptr <= '9') {
    ans = ans * 10 + (*ptr - '0');
    ptr++;
  }
  return is_neg ? -ans : ans;
}

static char *
skip_number(char *s)
{
  int digits = 0;

  if (*s == '-')
    s++;
  while (*s >= '0' && *s <= '9') {
    s++;
    digits++;
  }
  if (digits < 1 || digits > MAX_DIGITS)
    return 0;
  return s;
}

int
main(int argc, char *argv[])
{
  char buf[BUF_SIZE];

  int ret = read_line(buf, sizeof(buf));
  if (ret == -1) {
    fprintf(2, "Error: read failure\n");
    exit(1);
  }
  if (ret == -2) {
    fprintf(2, "Error: input is too long (max %d characters)\n", BUF_SIZE - 1);
    exit(1);
  }

  printf("|%s|\n", buf);

  if (buf[0] == '\0') {
    fprintf(2, "Error: empty input\n");
    exit(1);
  }

  char *end = skip_number(buf);
  if (end == 0 || *end != ' ') {
    fprintf(2, "Error: expected two integers (up to %d digits) separated by a single space\n", MAX_DIGITS);
    exit(1);
  }

  char *second = end + 1;
  end = skip_number(second);
  if (end == 0 || *end != '\0') {
    fprintf(2, "Error: expected two integers (up to %d digits) separated by a single space\n", MAX_DIGITS);
    exit(1);
  }

  printf("%d\n", my_atoi(buf) + my_atoi(second));
  exit(0);
}
