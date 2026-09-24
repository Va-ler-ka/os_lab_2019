#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct FactorialArgs {
  int begin;
  int end;
  int mod;
};

int result = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  long long part = 1;
  int i;

  for (i = fargs->begin; i <= fargs->end; i++) {
    part = (part * i) % fargs->mod;
  }

  pthread_mutex_lock(&mut);
  result = (int)((result * part) % fargs->mod);
  pthread_mutex_unlock(&mut);

  return NULL;
}

int main(int argc, char **argv) {
  int k = -1, pnum = -1, mod = -1;
  int i;

  while (1) {
    static struct option options[] = {{"pnum", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "k:", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        if (option_index == 0) pnum = atoi(optarg);
        if (option_index == 1) mod = atoi(optarg);
        break;
      case 'k':
        k = atoi(optarg);
        break;
      default:
        printf("Unknown option\n");
        return 1;
    }
  }

  if (k <= 0 || pnum <= 0 || mod <= 0) {
    printf("Usage: %s -k 10 --pnum=4 --mod=10\n", argv[0]);
    return 1;
  }

  pthread_t threads[pnum];
  struct FactorialArgs args[pnum];
  int step = k / pnum;

  for (i = 0; i < pnum; i++) {
    args[i].begin = i * step + 1;
    args[i].end = (i + 1) * step;
    args[i].mod = mod;
  }
  args[pnum - 1].end = k;

  for (i = 0; i < pnum; i++) {
    if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i]) != 0) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  for (i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("%d! mod %d = %d\n", k, mod, result);
  return 0;
}
