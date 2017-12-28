#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int count;
pthread_t *philosopher;
pthread_mutex_t *chopstick;

void random_sleep(void) {
  int t = rand() % 3;
  sleep(t + 1);
}

void eat(int n) {
  do {
    printf("Philosopher %d is thinking\n", n);
    random_sleep();

    printf("Philosopher %d is going to eat\n", n);
    if (n % 2 != 0) {
      pthread_mutex_lock(&chopstick[n]);
      pthread_mutex_lock(&chopstick[(n + 1) % 5]);
      printf("Philosopher %d is eating\n", n);
      pthread_mutex_unlock(&chopstick[n]);
      pthread_mutex_unlock(&chopstick[(n + 1) % 5]);
      printf("Philosopher %d finished eating\n", n);
    } else {
      pthread_mutex_lock(&chopstick[(n + 1) % 5]);
      pthread_mutex_lock(&chopstick[n]);
      printf("Philosopher %d is eating\n", n);
      pthread_mutex_unlock(&chopstick[(n + 1) % 5]);
      pthread_mutex_unlock(&chopstick[n]);
      printf("Philosopher %d finished eating\n", n);
    }

    random_sleep();
  } while (1);
}

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));
  if (argc != 2) {
    printf("Usage: %s <count>\n", argv[0]);
  }

  int i;
  count = atoi(argv[1]);
  chopstick = malloc(sizeof(pthread_mutex_t) * count);
  philosopher = malloc(sizeof(pthread_t) * count);
  for (i = 0; i < count; i++) {
    pthread_mutex_init(&chopstick[i], NULL);
  }

  for (i = 0; i < count; i++) {
    pthread_create(&philosopher[i], NULL, (void *)eat, (void *)(i + 1));
  }

  for (i = 0; i < count; i++) {
    pthread_join(philosopher[i], NULL);
  }

  for (i = 0; i < count; i++) {
    pthread_mutex_destroy(&chopstick[i]);
  }

  return 0;
}