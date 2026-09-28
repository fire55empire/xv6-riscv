#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int p[2], pid, i, n, w, r;
  char *s;

  if(pipe(p) < 0){
    printf("pipe failed\n");
    exit(1);
  }

  pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    if(close(0) < 0){
      printf("close failed\n");
      exit(1);
    }
    if(dup(p[0]) < 0){
      printf("dup failed\n");
      exit(1);
    }
    close(p[0]);
    close(p[1]);
    char *a[] = {"/wc", 0};
    exec("/wc", a);
    printf("exec /wc failed\n");
    exit(1);
  }

  close(p[0]);
  for(i = 1; i < argc; i++){
    s = argv[i];
    n = strlen(s);
    w = 0;
    while(w < n){
      r = write(p[1], s + w, n - w);
      if(r <= 0)
        break;
      w += r;
    }
    w = 0;
    while(w < 1){
      r = write(p[1], "\n" + w, 1 - w);
      if(r <= 0)
        break;
      w += r;
    }
  }
  close(p[1]);
  wait(0);
  exit(0);
}
