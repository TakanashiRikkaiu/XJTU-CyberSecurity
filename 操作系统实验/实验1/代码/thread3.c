#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int count = 0;          // 共享变量
sem_t mutex;            // 用于互斥访问
sem_t sync_signal;      // 用于同步控制

// 线程1：负责增加 count 并通知线程2
void* thread_inc(void* arg) {
    long tid = (long)pthread_self();
    printf("Thread %ld started (increment thread)\n", tid);

    for (int i = 0; i < 5; i++) {
        sem_wait(&mutex);       // P 操作：进入临界区
        count++;
        printf("Thread %ld: count = %d (incremented)\n", tid, count);
        sem_post(&mutex);       // V 操作：离开临界区

        sem_post(&sync_signal); // 通知线程2可以执行（同步）
        sleep(1);               // 模拟执行时间
    }

    return NULL;
}

// 线程2：等待线程1完成一次操作后再执行
void* thread_dec(void* arg) {
    long tid = (long)pthread_self();
    printf("Thread %ld started (decrement thread)\n", tid);

    for (int i = 0; i < 5; i++) {
        sem_wait(&sync_signal); // 等待线程1的信号（同步）
        sem_wait(&mutex);       // P 操作：进入临界区
        count--;
        printf("Thread %ld: count = %d (decremented)\n", tid, count);
        sem_post(&mutex);       // V 操作：离开临界区
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;

    sem_init(&mutex, 0, 1);         // 初始化互斥信号量（值=1）
    sem_init(&sync_signal, 0, 0);   // 初始化同步信号量（值=0）

    printf("Main PID = %d, initial count = %d\n", getpid(), count);

    pthread_create(&t1, NULL, thread_inc, NULL);
    pthread_create(&t2, NULL, thread_dec, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("All threads finished. Final count = %d\n", count);

    sem_destroy(&mutex);
    sem_destroy(&sync_signal);

    return 0;
}

