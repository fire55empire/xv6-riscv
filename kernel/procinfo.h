enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct procinfo {
  int  pid;
  char name[16];
  enum procstate state;
  int  ppid;
};

#define PS_ERR_TOOBIG (-1)
#define PS_ERR_FAULT  (-2)
