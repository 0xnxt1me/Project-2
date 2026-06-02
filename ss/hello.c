#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void dump_stack(void);
extern char* gets(char*); 

int main(int argc, char**argv) {
  char name[128];
  printf("%p\n", name);
  printf("What’s your name?\n");
  gets(name);
  dump_stack();
  printf("Hello ");
  printf(name);
  putchar('\n');
  fflush(stdout);
  // dump_stack();
  return 0;
}

