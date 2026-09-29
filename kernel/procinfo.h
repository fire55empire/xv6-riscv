struct procinfo {
  int  pid;
  char name[16];
  int  state;
  int  ppid;
};

#define PS_ERR_TOOBIG (-1)
#define PS_ERR_FAULT  (-2)
