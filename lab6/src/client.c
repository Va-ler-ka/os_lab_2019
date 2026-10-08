#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include "multmodulo.h"


struct Server {
  char ip[255];
  int port;
};


struct ServerArgs {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};



bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void *ServerThread(void *args) {
  struct ServerArgs *sargs = (struct ServerArgs *)args;

  struct hostent *hostname = gethostbyname(sargs->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", sargs->server.ip);
    exit(1);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(sargs->server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    exit(1);
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    exit(1);
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &sargs->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &sargs->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &sargs->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    exit(1);
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Recieve failed\n");
    exit(1);
  }

  memcpy(&sargs->result, response, sizeof(uint64_t));
  printf("server %s:%d -> %llu\n", sargs->server.ip, sargs->server.port,
         (unsigned long long)sargs->result);

  close(sck);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        if (k == 0) {
          printf("k must be positive\n");
          return 1;
        }
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        if (mod == 0) {
          printf("mod must be positive\n");
          return 1;
        }
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file
  FILE *f = fopen(servers, "r");
  if (f == NULL) {
    fprintf(stderr, "Can not open file %s\n", servers);
    return 1;
  }

  // first pass: count servers in file
  unsigned int servers_num = 0;
  char ip[255];
  int port;
  while (fscanf(f, "%254[^:]:%d\n", ip, &port) == 2) {
    servers_num++;
  }

  if (servers_num == 0) {
    fprintf(stderr, "No servers in file %s\n", servers);
    fclose(f);
    return 1;
  }

  // second pass: read servers into array
  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  rewind(f);
  for (int i = 0; i < servers_num; i++) {
    fscanf(f, "%254[^:]:%d\n", to[i].ip, &to[i].port);
  }
  fclose(f);

  // TODO: work continiously, rewrite to make parallel
  // one thread for each server, all servers work at the same time
  pthread_t threads[servers_num];
  struct ServerArgs args[servers_num];
  uint64_t step = k / servers_num;

  for (int i = 0; i < servers_num; i++) {
    args[i].server = to[i];
    args[i].begin = i * step + 1;
    args[i].end = (i + 1) * step;
    if (i == servers_num - 1)
      args[i].end = k;
    args[i].mod = mod;
    args[i].result = 1;

    if (pthread_create(&threads[i], NULL, ServerThread, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  // unite results
  uint64_t answer = 1;
  for (int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    answer = MultModulo(answer, args[i].result, mod);
  }

  printf("answer: %llu\n", (unsigned long long)answer); 




  free(to);

  return 0;
}
