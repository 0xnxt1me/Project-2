#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void dump_stack(void);
// Removed dangerous extern gets() declaration

int main(int argc, char**argv) {
  char name[128];
  printf("%p\n", name);
  printf("What’s your name?\n");
  fgets(name, sizeof(name), stdin);
  dump_stack();
  printf("Hello ");
  printf("%s", name);
  putchar('\n');
  fflush(stdout);
  // dump_stack();
  return 0;
}

