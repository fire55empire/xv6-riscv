#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
printargs(int argc, char *argv[], int mfd)
{
  int i;
  char *s;

  for (i = 1; i < argc; i++) {
    for (s = argv[i]; *s; s++) {
      if (mfd >= 0)
        mutex_lock(mfd);
      printf("%d: arg %d, char '%c'\n", getpid(), i, *s);
      if (mfd >= 0)
        mutex_unlock(mfd);
    }
  }
}

int
main(int argc, char *argv[])
{
  int fd, pid1, pid2;

  if (argc < 2) {
    printf("usage: mutextest arg1 [arg2 ...]\n");
    exit(1);
  }

  printf("without synchronization:\n");

  pid1 = fork();
  if (pid1 == 0) {
    printargs(argc, argv, -1);
    exit(0);
  }
  pid2 = fork();
  if (pid2 == 0) {
    printargs(argc, argv, -1);
    exit(0);
  }
  wait(0);
  wait(0);

  printf("with mutex synchronization:\n");

  fd = mutex();
  if (fd < 0) {
    printf("mutex() failed\n");
    exit(1);
  }

  pid1 = fork();
  if (pid1 == 0) {
    printargs(argc, argv, fd);
    exit(0);
  }
  pid2 = fork();
  if (pid2 == 0) {
    printargs(argc, argv, fd);
    exit(0);
  }
  wait(0);
  wait(0);
  close(fd);

  exit(0);
}
