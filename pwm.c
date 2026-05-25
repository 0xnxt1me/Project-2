#include <stdio.h>
#include <errno.h>
#include "pwm.h"

int main(int argc, char** argv) {
  --argc; ++argv;
  if (argc == 1) {
    if (freopen(argv[0], "r", stdin) == 0) {
      perror(argv[0]);
      return 1;
    }
  }
  pwm_interactive_session();
  return 0;
}
