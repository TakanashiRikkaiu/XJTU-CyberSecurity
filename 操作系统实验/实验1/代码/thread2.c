#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int count = 0;                 // 共享变量
pthread_mutex_t lock;          // 定义互斥锁

void* thread_inc(void* arg) {  // 线程1：自增
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 60000; i++) {
        pthread_mutex_lock(&lock);   // 加锁
        count++;                     // 临界区
        pthread_mutex_unlock(&lock); // 解锁
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

void* thread_dec(void* arg) {  // 线程2：自减
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 60000; i++) {
        pthread_mutex_lock(&lock);   // 加锁
        count--;                     // 临界区
        pthread_mutex_unlock(&lock); // 解锁
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

int main() {
    pthread_t t1, t2;

    pthread_mutex_init(&lock, NULL); // 初始化互斥锁

    printf("Main process PID = %d, initial count = %d\n", getpid(), count);

    pthread_create(&t1, NULL, thread_inc, NULL);
    pthread_create(&t2, NULL, thread_dec, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Process finished, final count = %d\n", count);

    pthread_mutex_destroy(&lock);    // 销毁互斥锁
    return 0;
}

