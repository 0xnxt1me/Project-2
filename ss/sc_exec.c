#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>
#include <alloca.h>

int main(int argc, char** argv) {
  --argc; ++argv;

  if (argc != 1) {
    fprintf(stderr, "Usage: sc_exec file\n");
    return 1;
  }
  
  const char* file = *argv;
  struct stat st;


  FILE* fp = fopen(file, "r");

  if (!fp || fstat(fileno(fp), &st) != 0) {
   perror(file);
   return 1;
  }

  uint8_t* instructions = (uint8_t*) alloca(st.st_size);
  if (!instructions) {
    perror("alloca");
    return 1;
  }
  
  if (fread(instructions, 1, st.st_size, fp) != st.st_size) {
    perror("alloca");
    return 1;
  }

  if (!fp) {
    perror(*argv);
    return 1;
  }

  printf("%s: read instructions (%ld bytes) ...\n", file, st.st_size);
  for (uint8_t* p = instructions, *p_e = instructions + st.st_size; p < p_e; p++) {
    printf("%02x", *p);
  }
  putchar('\n');
  fflush(stdout);
  (*(void (*)()) instructions)();
  return 0;
}
