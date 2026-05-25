#include <stdio.h>
#include <ctype.h>
  
static void dump_word(void* p) {
  unsigned char* cp = (unsigned char*) p;
  unsigned char* cp_e = cp + sizeof(void*);
  while (cp != cp_e) {
    if (isprint(*cp)) {
      printf("  %c", *cp);
    } 
    else {
      printf(" %02x", *cp);
    }
    ++cp;
  }
}

#define DUMP(id) { \
  printf("%3d %3d | %p --> %-18p %2s | ", \
         (int) ((p-base) * sizeof(void*)), \
         (int) ((rbp-p) * sizeof(void*)), \
         p, \
         *p, \
         id); \
  dump_word(p); \
  putchar('\n'); \
  p++; \
}

void dump_stack(void) {
  void** base, **rbp, **p; 
  __asm__ __volatile__("movq %%rbp,%0" :"=r"(base));
  __asm__ __volatile__("movq 0(%%rbp),%0" :"=r"(rbp));

  base+=2;
  p = base;

  while (p < rbp) {
    DUMP("");
  }
  DUMP("FP");
  DUMP("RA");
}


