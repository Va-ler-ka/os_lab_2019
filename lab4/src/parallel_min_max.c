#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

// Глобальные переменные, чтобы обработчик сигнала их видел
int pnum_global = 0;
pid_t child_pids[1000];

// Обработчик SIGALRM: убиваем всех детей
void kill_children(int signum) {
  printf("Timeout! Killing child processes...\n");
  for (int i = 0; i < pnum_global; i++) {
    if (child_pids[i] > 0) {
      kill(child_pids[i], SIGKILL);
    }
  }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed is a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size is a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum is a positive number\n");
              return 1;
            }
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout is a positive number\n");
              return 1;
            }
            break;
          case 4:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf(
        "Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" "
        "[--timeout \"num\"]\n",
        argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  int pipefd[pnum][2];

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    if (!with_files) {
      if (pipe(pipefd[i]) == -1) {
        printf("Pipe failed!\n");
        return 1;
      }
    }

    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process

        // Раскомментировать, чтобы проверить работу таймаута:
        sleep(10);

        int chunk_size = array_size / pnum;
        int begin = i * chunk_size;
        int end = (i == pnum - 1) ? array_size : begin + chunk_size;

        struct MinMax min_max = GetMinMax(array, begin, end);

        if (with_files) {
          char filename[256];
          sprintf(filename, "min_max_%d.txt", i);
          FILE *fp = fopen(filename, "w");
          fprintf(fp, "%d %d", min_max.min, min_max.max);
          fclose(fp);
        } else {
          close(pipefd[i][0]);
          write(pipefd[i][1], &min_max, sizeof(min_max));
          close(pipefd[i][1]);
        }
        return 0;
      } else {
        // parent process
        child_pids[i] = child_pid;
        if (!with_files) {
          close(pipefd[i][1]);
        }
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  // Если таймаут задан - ставим будильник
  if (timeout > 0) {
    pnum_global = pnum;
    signal(SIGALRM, kill_children);
    alarm(timeout);
  }

  // Неблокирующее ожидание детей
  while (active_child_processes > 0) {
    pid_t pid = waitpid(-1, NULL, WNOHANG);
    if (pid > 0) {
      active_child_processes -= 1;
    }
  }

  // Если дети завершились сами - будильник больше не нужен
  alarm(0);

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[256];
      sprintf(filename, "min_max_%d.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp == NULL) {
        printf("Child %d was killed, no result\n", i);
        continue;
      }
      fscanf(fp, "%d %d", &min, &max);
      fclose(fp);
    } else {
      struct MinMax tmp;
      if (read(pipefd[i][0], &tmp, sizeof(tmp)) <= 0) {
        printf("Child %d was killed, no result\n", i);
        close(pipefd[i][0]);
        continue;
      }
      close(pipefd[i][0]);
      min = tmp.min;
      max = tmp.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
