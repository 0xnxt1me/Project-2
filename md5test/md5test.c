
#include <stdio.h>
#include <stdlib.h>
#include "md5.h"

int main(int argc, char** argv) {
  --argc; ++argv;
  if (argc == 0) {
    fprintf(stderr, "Usage: md5 file1 ... filen\n");
    return 1;
  }

  do {
    char* filename = *argv;
    FILE *fp = fopen (filename, "r");
    if (!fp) {
      perror(filename);
      return 1;
    }
    unsigned char buf[4096];
    int n;
    MD5_CTX ctx;
    MD5Init(&ctx);
    unsigned char checksum[16]; 

    do {
      n = fread(buf, 1, sizeof(buf), fp); 
      if (n < 0) {
        perror(filename);
        fclose(fp);
        return 1;
      } else if (n > 0) {
        MD5Update(&ctx, buf, n);  
      }      
    } while (n == sizeof(buf));
    MD5Final(checksum, &ctx);
    printf("MD5 (%s) = ", filename);
    for (int i = 0; i < sizeof(checksum); i++) {
      printf("%02x", checksum[i]);
    }
    putchar('\n');
    fflush(stdout);
    ++argv;
    --argc;
  } while (argc > 0);
  return 0;
}
