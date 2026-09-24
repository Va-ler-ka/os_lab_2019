#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();

  if (pid < 0) {
    printf("Fork failed!\n");
    return 1;
  }

  if (pid == 0) {
    // Дочерний процесс сразу завершается
    printf("Child: my pid is %d, I am exiting now\n", getpid());
    return 0;
  } else {
    // Родитель не вызывает wait() и продолжает работать
    printf("Parent: my pid is %d, child pid is %d\n", getpid(), pid);
    printf("Child is a zombie now. Check it: ps -el | grep defunct\n");
    sleep(20);

    // Забираем статус ребенка - зомби исчезает
    wait(NULL);
    printf("Parent: wait() is done, zombie is gone. Check again.\n");
    sleep(20);
  }

  return 0;
}
