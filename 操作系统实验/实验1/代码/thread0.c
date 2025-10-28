#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int count = 0;  // 共享变量

void* thread_inc(void* arg) {  // 线程1：自增
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 60000; i++) {
        count++;  // 加1操作
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

void* thread_dec(void* arg) {  // 线程2：自减
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 60000; i++) {
        count--;  // 减1操作
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

int main() {
    pthread_t t1, t2;

    printf("Main process PID = %d, initial count = %d\n", getpid(), count);

    pthread_create(&t1, NULL, thread_inc, NULL);
    printf("Thread 1 created (TID = %ld)\n", (long)t1);

    pthread_create(&t2, NULL, thread_dec, NULL);
    printf("Thread 2 created (TID = %ld)\n", (long)t2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("process finished, final count = %d\n", count);

    return 0;
}

