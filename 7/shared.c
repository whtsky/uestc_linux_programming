#include "shared.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>

key_t get_buf_key(void) { return ftok(".", BUF_PROJ_ID); }

key_t get_sem_key(void) { return ftok(".", SEM_PROJ_ID); }

void terminator(void) {
  pthread_detach(pthread_self());
  while (getchar() != 'q') {
    ;
  }
  exit(EXIT_SUCCESS);
}

void run_terminator(void) {
  pthread_t pid;
  pthread_create(&pid, NULL, (void *(*)(void *))terminator, NULL);
}

BufferPool *get_buffer_pool(void) {
  bool init;
  int shmid = shmget(get_buf_key(), sizeof(BufferPool),
                     IPC_EXCL | IPC_CREAT | IPC_R | IPC_W | IPC_M);
  if (shmid == -1) {
    if (errno == EEXIST) {
      shmid = shmget(get_buf_key(), sizeof(BufferPool),
                     IPC_CREAT | IPC_R | IPC_W | IPC_M);
      printf("get shared memory %d\n", shmid);
      init = false;
    } else {
      perror("shmget");
      exit(EXIT_FAILURE);
    }
  } else {
    printf("created shared memory %d\n", shmid);
    init = true;
  }
  BufferPool *poll = shmat(shmid, 0, 0);
  if (poll == (void *)-1) {
    perror("shm");
    exit(EXIT_FAILURE);
  }
  if (init == true) {
    for (int i = 0; i < POLL_LENGTH; i++) {
      poll->written[i] = false;
    }
  }
  return poll;
}

int SEM_ID = -1;

int get_semset(void) {
  if (SEM_ID == -1) {
    int semid = semget(get_sem_key(), 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (semid == -1) {
      if (errno == EEXIST) {
        SEM_ID = semget(get_sem_key(), 1, 0666);
        if (SEM_ID == -1) {
          perror("semget");
          exit(EXIT_FAILURE);
        }
        printf("get sem %d\n", SEM_ID);
        return SEM_ID;
      }
      perror("semget");
      exit(EXIT_FAILURE);
    }
    SEM_ID = semid;
    printf("created sem %d\n", semid);
    // set default value to 1
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(SEM_ID, &sem_b, 1) == -1) {
      perror("semop");
      exit(EXIT_FAILURE);
    }
    puts("init sem to 1");
  }
  return SEM_ID;
}

bool semaphore_v(void) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = 1;
  sem_b.sem_flg = SEM_UNDO;
  if (semop(get_semset(), &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return false;
  }
  return true;
}

bool semaphore_p(void) {
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1;
  sem_b.sem_flg = SEM_UNDO;
  if (semop(get_semset(), &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_p failed\n");
    return false;
  }
  return true;
}

void random_sleep(void) {
  int t = rand() % 3;
  sleep(t + 1);
}

void producer(void) {
  FILE *file = fopen("producer.txt", "r");
  if (file == NULL) {
    perror("producer open");
    exit(EXIT_FAILURE);
  }
  BufferPool *poll = get_buffer_pool();
  while (true) {
  producer_loop:
    semaphore_p();
    for (int i = 0; i < POLL_LENGTH; i++) {
      if (poll->written[i] == false) {
        printf("Found available buf: %d\n", i);
        // available, write here
        if (feof(file)) {
          rewind(file);
        }
        fgets(poll->buffer[i], 99, file);
        printf("Wrote to %d\n", i);
        poll->written[i] = true;
        semaphore_v();
        random_sleep();
        goto producer_loop;
      }
    }
    puts("No buffer available to write, sleep");
    semaphore_v();
    random_sleep();
  }
}

void consumer(int id) {
  char buf[100];
  sprintf(buf, "consumer%d.txt", id);
  int fd = open(buf, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1) {
    perror("consumer open");
    exit(EXIT_FAILURE);
  }
  BufferPool *poll = get_buffer_pool();
  while (true) {
  consumer_loop:
    semaphore_p();
    for (int i = 0; i < POLL_LENGTH; i++) {
      if (poll->written[i] == true) {
        write(fd, poll->buffer[i], strlen(poll->buffer[i]));
        printf("found data: %s", poll->buffer[i]);
        poll->written[i] = false;
        semaphore_v();
        random_sleep();
        goto consumer_loop;
      }
    }
    puts("no data available, sleep");
    semaphore_v();
    random_sleep();
  }
}