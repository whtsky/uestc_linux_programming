#include "cp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source> <target>", argv[0]);
    exit(EXIT_FAILURE);
  }
  cp(argv[1], argv[2]);
  return 0;
}
