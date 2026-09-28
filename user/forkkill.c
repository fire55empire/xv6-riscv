#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int pid, cpid, st, t;

  t = 90 + (getpid() % 60);
  pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }
  if(pid == 0){
    pause(t);
    exit(1);
  }
  printf("parent %d child %d\n", getpid(), pid);
  cpid = wait(&st);
  printf("wait done, pid=%d status=%d\n", cpid, st);

  t = 90 + (getpid() % 60);
  pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }
  if(pid == 0){
    pause(t);
    exit(1);
  }
  printf("parent %d child %d\n", getpid(), pid);
  kill(pid);
  cpid = wait(&st);
  printf("kill done, pid=%d status=%d\n", cpid, st);

  exit(0);
}
