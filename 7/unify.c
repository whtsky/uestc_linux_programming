#include "shared.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pid_t child_pid = -1;

void terminate_family(void) {
  pthread_detach(pthread_self());
  while (getchar() != 'q') {
    ;
  }
  if (child_pid != -1) {
    kill(child_pid, SIGKILL);
  }
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <consumer_id>", argv[0]);
    exit(EXIT_FAILURE);
  }
  pthread_t pid;
  pthread_create(&pid, NULL, (void *(*)(void *))terminate_family, NULL);
  if (fork() == 0) {
    consumer(atoi(argv[1]));
  } else {
    producer();
  }
}