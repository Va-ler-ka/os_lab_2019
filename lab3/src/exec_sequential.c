#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  pid_t pid = fork();
  if (pid == 0) {
    // дочерний процесс — заменяем его на sequential_min_max
    execl("./sequential_min_max", "sequential_min_max", "1", "100", NULL);
    // если execl вернул управление, значит была ошибка
    printf("exec failed!\n");
    return 1;
  } else if (pid > 0) {
    // родитель ждёт завершения потомка
    wait(NULL);
  } else {
    printf("Fork failed!\n");
    return 1;
  }
  return 0;
}
