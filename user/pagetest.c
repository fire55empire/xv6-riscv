#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define AD (PTE_A | PTE_D)
#define HEAPSZ (3 * 4096)

int gvar = 7;

void
flags(char *name, void *p, int n)
{
  printf("  %s %p: A=%d D=%d\n", name, p, pgcheck(p, n, PTE_A),
         pgcheck(p, n, PTE_D));
}

void
allflags(char *heap, int *svar, int *sarr)
{
  flags("global", &gvar, sizeof(gvar));
  flags("stack var", svar, sizeof(int));
  flags("stack arr", sarr, 4 * sizeof(int));
  flags("heap arr", heap, HEAPSZ);
}

int
main(void)
{
  int svar = 11;
  int sarr[4];
  char *heap;
  int i, x = 0;

  printf("=== start ===\n");
  pagetable();

  heap = sbrk(HEAPSZ);
  for (i = 0; i < HEAPSZ; i++)
    heap[i] = i;
  printf("=== after alloc, heap=%p ===\n", heap);
  pagetable();

  pgclear(0, (int)(uint64)sbrk(0), AD);
  printf("=== after clear A and D ===\n");
  pagetable();
  allflags(heap, &svar, sarr);

  x += gvar;
  x += svar;
  x += sarr[1];
  x += heap[0] + heap[4096] + heap[8192];
  printf("=== after read (x=%d) ===\n", x);
  pagetable();
  allflags(heap, &svar, sarr);

  pgclear(0, (int)(uint64)sbrk(0), AD);
  gvar = 1;
  svar = 2;
  sarr[2] = 3;
  heap[5000] = 4;
  printf("=== after write ===\n");
  pagetable();
  allflags(heap, &svar, sarr);

  sbrk(-HEAPSZ);
  printf("=== after free ===\n");
  pagetable();

  printf("bad range: %d\n", pgcheck(heap, 100, PTE_A));
  printf("bad mask: %d\n", pgclear(&gvar, 4, PTE_R));
  exit(0);
}
