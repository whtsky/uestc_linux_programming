#include "shared.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <consumer_id>", argv[0]);
    exit(EXIT_FAILURE);
  }
  run_terminator();
  consumer(atoi(argv[1]));
}