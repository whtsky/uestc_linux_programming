#pragma once

#include <stdbool.h>
#include <sys/shm.h>

#define BUF_PROJ_ID 2233
#define SEM_PROJ_ID 2234
#define POLL_LENGTH 5

typedef struct {
  // false 表示对应的缓冲区未被生产者使用，可分配但不可消费；true
  // 表示对应的缓冲区以被生产者使用，不可分配但可消费
  bool written[POLL_LENGTH];
  //字符串缓冲区
  char buffer[POLL_LENGTH][100];
} BufferPool;

void producer(void);
void consumer(int id);
void run_terminator(void);