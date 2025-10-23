#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    printf("parent process PID: %d\n", getpid());

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        return 1;
    }
    else if (pid == 0) {
        printf("child process PID: %d\n", getpid());
        system("./system_call");
    }
    else {
        wait(NULL);
        printf("child process finished. parent PID: %d\n", getpid());
    }

    return 0;
}
