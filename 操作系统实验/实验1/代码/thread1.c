#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int count = 0;          // 共享变量
sem_t signal;           // 定义信号量

void* thread_inc(void* arg) {
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 60000; i++) {
        sem_wait(&signal);  // P 操作，申请资源
        count++;            // 临界区
        sem_post(&signal);  // V 操作，释放资源
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

void* thread_dec(void* arg) {
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 60000; i++) {
        sem_wait(&signal);  // P 操作
        count--;            // 临界区
        sem_post(&signal);  // V 操作
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

int main() {
    pthread_t t1, t2;

    sem_init(&signal, 0, 1);  // 初始化信号量为1，表示可用资源=1（互斥锁效果）

    printf("Main process PID = %d, initial count = %d\n", getpid(), count);

    pthread_create(&t1, NULL, thread_inc, NULL);
    pthread_create(&t2, NULL, thread_dec, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Process finished, final count = %d\n", count);

    sem_destroy(&signal);  // 销毁信号量
    return 0;
}

