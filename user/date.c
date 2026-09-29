#include "kernel/types.h"
#include "user/user.h"

int
is_leap(int year)
{
  if (year % 400 == 0)
    return 1;
  if (year % 100 == 0)
    return 0;
  if (year % 4 == 0)
    return 1;
  return 0;
}

int
days_in_month(int year, int month)
{
  int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month == 2 && is_leap(year))
    return 29;
  return days[month - 1];
}

void
print_num(uint64 n, int width)
{
  char buf[20];
  int len = 0;

  while (n > 0) {
    buf[len] = '0' + n % 10;
    len++;
    n = n / 10;
  }
  for (int i = len; i < width; i++)
    printf("0");
  while (len > 0) {
    len--;
    printf("%c", buf[len]);
  }
}

int
main(void)
{
  uint64 ns = rtc();
  uint64 seconds = ns / 1000000000;
  uint64 nanos = ns % 1000000000;
  uint64 days = seconds / 86400;
  uint64 day_seconds = seconds % 86400;

  int hour = day_seconds / 3600;
  int minute = day_seconds % 3600 / 60;
  int second = day_seconds % 60;

  int year = 1970;
  while (days >= 365 + is_leap(year)) {
    days = days - 365 - is_leap(year);
    year++;
  }

  int month = 1;
  while (days >= days_in_month(year, month)) {
    days = days - days_in_month(year, month);
    month++;
  }
  int day = days + 1;

  print_num(year, 4);
  printf("-");
  print_num(month, 2);
  printf("-");
  print_num(day, 2);
  printf(" ");
  print_num(hour, 2);
  printf(":");
  print_num(minute, 2);
  printf(":");
  print_num(second, 2);
  printf(".");
  print_num(nanos, 9);
  printf("\n");
  exit(0);
}
