#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

void *thread_one(void *arg) {
  printf("Thread 1: lock A\n");
  pthread_mutex_lock(&mutex_a);

  sleep(1);

  printf("Thread 1: want B\n");
  pthread_mutex_lock(&mutex_b);

  printf("Thread 1: got A and B\n");
  pthread_mutex_unlock(&mutex_b);
  pthread_mutex_unlock(&mutex_a);
  return NULL;
}

void *thread_two(void *arg) {
  printf("Thread 2: lock B\n");
  pthread_mutex_lock(&mutex_b);

  sleep(1);

  printf("Thread 2: want A\n");
  pthread_mutex_lock(&mutex_a);

  printf("Thread 2: got B and A\n");
  pthread_mutex_unlock(&mutex_a);
  pthread_mutex_unlock(&mutex_b);
  return NULL;
}

int main() {
  pthread_t t1, t2;

  pthread_create(&t1, NULL, thread_one, NULL);
  pthread_create(&t2, NULL, thread_two, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("Done\n");
  return 0;
}
