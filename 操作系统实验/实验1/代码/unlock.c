/**
 * spinlock_no_lock.c (无锁版本)
 * in xjtu
 * 2023.8
 * 修改：去掉自旋锁保护，演示竞态条件
 */
#include <stdio.h>
#include <pthread.h>

// 共享变量
int shared_value = 0;

// 线程函数（去掉锁）
void *thread_function(void *arg) {
    // 注意：arg 被忽略，因为不再需要锁
    for (int i = 0; i < 5000; ++i) {
        // 无锁保护：shared_value++ 不是原子操作
        // 这会导致竞态条件：两个线程可能同时读到相同值，导致递增丢失
        shared_value++;
    }
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    // 注意：无需初始化锁

    // 输出初始共享变量值
    printf("Before: shared_value = %d\n", shared_value);

    // 创建两个线程（传递 NULL，因为 arg 未使用）
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);

    // 等待两个线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // 输出最终共享变量值
    printf("After: shared_value = %d\n", shared_value);

    return 0;
}
