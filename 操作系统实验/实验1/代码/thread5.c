#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

void* thread_call(void* arg) {
    long id = (long)arg;
    printf("thread%ld created\n", id);
    printf("thread%ld tid = %ld ,pid = %d\n", id, pthread_self(), getpid());

    pid_t pid = fork();
    if (pid == 0) {
        execl("./system_call", "system_call", NULL);
        perror("execl failed");
        exit(1);
    } else {
        wait(NULL);
        printf("thread%ld systemcall return\n", id);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_call, (void*)1);
    pthread_create(&t2, NULL, thread_call, (void*)2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}

