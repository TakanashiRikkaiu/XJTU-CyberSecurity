我是傻逼     makefile？是什么
## 实验一     



# 1.1运行课本程序
由于所给程序使用了wait函数且缺少头文件，在使用命令行编译时会报错：
<div align="center">
  <img width="1685" height="174" alt="image" src="https://github.com/user-attachments/assets/1bcf9792-5d23-467b-ac2e-23d05e967d4d" />
</div>


对于所给程序，刚开始时我为了观察输入方便，在每个输出内加入了换行符\n,所输出的结果如下图所示：
<div align="center">
 <img width="571" height="704" alt="image" src="https://github.com/user-attachments/assets/4727eafb-6309-4f92-b41c-38862ffada5b" />
</div>
<div align="center">
  <img width="524" height="704" alt="image" src="https://github.com/user-attachments/assets/1e31b4a8-2c92-4235-9636-cb6893d52cae" />
</div>


如图所示，每次的输出分为4行，易于观察，但是奇怪的是输出的顺序。由于wait函数的影响，我认为应该是先输出子进程的两个pid，而后才是父进程，就像这样：
child: pid = 0  
child: pid1 = 4890  
parent: pid = 4890  
parent: pid1 = 4889  
然而我在安装了基于OpenEuler的Deepin桌面系统的虚拟机上对相同的程序进行编译运行，结果却是这样的：
<img width="526" height="684" alt="image" src="https://github.com/user-attachments/assets/cee314d3-27cc-4b1e-9c15-e1f550cf22ca" />
<img width="536" height="676" alt="image" src="https://github.com/user-attachments/assets/b8e26388-a37d-462a-99bc-d1a70910f7ec" />
这次则是先输出了父进程，然后才是子进程。
查阅资料得知存在三种情况：
情况 1：子进程先执行（常见）
child: pid = 0
child: pid1 = 1234
parent: pid = 1234
parent: pid1 = 1233
情况 2：父进程先执行其打印（然后等待子进程）
parent: pid = 1234
parent: pid1 = 1233
child: pid = 0
child: pid1 = 1234
其他可能：打印可能交错，例如：
child: pid = 0
parent: pid = 1234
child: pid1 = 1234
parent: pid1 = 1233
查询资料得知，输出顺序取决于操作系统调度，具体原因：父进程和子进程是并发执行的（parallel execution）。操作系统内核的**调度器（scheduler）**决定哪个进程先运行，这取决于 CPU 负载、调度算法（通常是 CFS - Completely Fair Scheduler）和随机因素（如时钟中断）
实际上，代码中每个 printf 都有 \n，所以理论上每行独立刷新，但如果缓冲区共享或切换时机巧合，仍可能部分重叠。这应该是主要原因。
删除各行的\n后再次使用华为云服务器进行编译运行，得到了正确的输出顺序：
代码块？
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    pid_t pid, pid1;

    /* fork a child process */
    pid = fork();

    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { /* child process */
        pid1 = getpid();
        printf("child: pid = %d\n", pid);   /* A */
        printf("child: pid1 = %d\n", pid1); /* B */
    }
    else { /* parent process */
        pid1 = getpid();
        printf("parent: pid = %d\n", pid);   /* C */
        printf("parent: pid1 = %d\n", pid1); /* D */
        wait(NULL);
    }

    return 0;
}
pid改
<img width="1455" height="317" alt="image" src="https://github.com/user-attachments/assets/7ae545c1-58a6-4562-bf4d-ccef3ea02a46" />

初步想法运行几十次获得统计性的一般规律，限于篇幅仅运行10次，可以得到结果规律如下：
？？？？？？？？？？？？？？？/


在华为云和虚拟机分别运行
结果：
要不要加入头文件试试

去除wait的结果：
pid1改图片
<img width="1476" height="282" alt="image" src="https://github.com/user-attachments/assets/9c1ab1ef-8303-4083-8cb5-979be170ef3e" />

按照此逻辑，如果使用了wait且包含了头文件，应该是成功调用函数，父等子结束。
结果分析：
1.2情况对比。

*2全局变量
补全头文件后，添加一个全局变量g，代码如下；
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int g = 10;   

int main()
{
    pid_t pid, pid1;
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) {  
        g += 5;  
        pid1 = getpid();
        printf("child: pid = %d\n", pid);
        printf("child: pid1 = %d, g = %d\n", pid1, g);
    }
    else {  
        g -= 5;  
        pid1 = getpid();
        printf("parent: pid = %d\n", pid);
        printf("parent: pid1 = %d, g = %d\n", pid1, g);
        wait(NULL);
    }
    return 0;
}
子进程自加，父进程自减。
思考：


图片：
<img width="585" height="176" alt="image" src="https://github.com/user-attachments/assets/7c5d1165-f78a-4962-a23f-b0ccbba0ce45" />



3.return
代码
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int g = 10;   

int main()
{
    pid_t pid, pid1;
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) {  
        g += 5;  
        pid1 = getpid();
        printf("child: pid1 = %d, g(after +5) = %d\n", pid1, g);
    }
    else {  
        g -= 5;  
        pid1 = getpid();
        printf("parent: pid1 = %d, g(after -5) = %d\n", pid1, g);
        wait(NULL);
    }


    g *= 2;  
    printf("pid %d: g(after *2) = %d\n", getpid(), g);

    return 0;
}

可得
gl图片
<img width="880" height="201" alt="image" src="https://github.com/user-attachments/assets/2bb70819-7433-43b9-929d-4b2aaca9595b" />


aa4
编写被调用程序：
代码
系统调用图
<img width="878" height="90" alt="image" src="https://github.com/user-attachments/assets/12274c08-0bdf-4bc1-b524-22297ba0b680" />
不支持中文输出：我的学号，出现乱码。

两个程序和图片
system：
<img width="855" height="200" alt="image" src="https://github.com/user-attachments/assets/93637897-dd5c-4f84-a2e0-694a15cd1eb0" />





exec：
<img width="665" height="179" alt="image" src="https://github.com/user-attachments/assets/09206440-12c2-41df-875d-4359e2737c30" />



分析结果。

1.2线程实验
开始时，设计程序，创建两个子线程，两线程分别对同一个共享变量多次操作，观察输出结果。
选择次数6000
加锁or not？？？尝试
运行十余次，出现了多次现象，如图所示



4图

是的 ✅ ——你这个判断非常到位。

从**理论上讲**，在没有同步机制（锁、原子操作等）的情况下，两线程对同一个共享变量进行修改时，**结果应该是不确定的**，而且**几乎不可能完全等于理论值**（12000）。

但是在**现实运行中（尤其是循环较少，比如6000次）**，往往会出现一种“假象”——你运行了几次，结果反而一直是12000。
这其实是因为：

---

### 🧩 一、现代系统下“偶然正确”的原因

1. **线程创建与调度延迟**
   两个线程虽然几乎同时创建，但调度器往往会让一个线程先执行完大部分循环，再切换到另一个线程。
   → 也就是“伪并行”，顺序执行，所以结果没错。

2. **循环次数太少**
   6000次在现代 CPU 上只需要几微秒完成。
   调度切换（线程间切换）往往需要上万微秒级别，线程可能还没被切换就已经跑完了。

3. **系统负载低**
   若此时 CPU 空闲，操作系统就不会频繁打断线程调度。
   两个线程各跑完一半，就好像是顺序执行一样。

---

### ⚙️ 二、结论

所以你说的“**主观上应该不太可能完全等于理论值**”是正确的，
但在现实中——**由于系统调度特性**，
**小循环 + 空闲CPU → 实际上经常出现理论值**。

---

### 🔬 想亲眼看到竞争？

你可以这样改：

```c
for (int i = 0; i < 6000; i++) {
    int temp = count;
    usleep(1);  // 故意延迟1微秒，让调度有机会切换
    count = temp + 1;
}
```

这样几乎一定不会得到12000。

---

要不要我帮你解释下 **加上 `usleep(1)` 后为什么几乎必定错**？这能更直观理解竞争条件的本质。

增加操作数☞60000
代码
4图，此时次次输出均与理论值不符。分析：

；增加信号量和pv操作，再次运行10次结果如下
图3*

3 ## 可得输出稳定为0
；使用互斥锁。


尝试灵活运用信号量和PV 操作实现线程间的同步互斥。？？？？？？？？？？？？？？？？？？？
综合运用，2张图
分析pid关系

2种方法系统调用
ok
由于之前编写system_call.c时在代码中使用中文。而终端中输出似乎并不支持，于是修改代码，再次运行如下：
sys改png

修改完代码，编写程序，在进程中创建两个线程，分别使用两个函数方法进行系统调用
①system()
代码：



结果：
systhread


②:exec
代码：

结果：

图片



1.3自旋锁实验
补全代码如下：
/**
* spinlock.c
* in xjtu
* 2025.10
*/
#include <stdio.h>
#include <pthread.h>

// 定义自旋锁结构体
typedef struct {
    int flag;
} spinlock_t;

// 初始化自旋锁
void spinlock_init(spinlock_t *lock) {
    lock->flag = 0;
}

// 获取自旋锁
void spinlock_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(&lock->flag, 1)) {
        // 自旋等待，直到flag为0时才退出循环
    }
}

// 释放自旋锁
void spinlock_unlock(spinlock_t *lock) {
    __sync_lock_release(&lock->flag);
}

// 共享变量
int shared_value = 0;

// 线程函数
void *thread_function(void *arg) {
    spinlock_t *lock = (spinlock_t *)arg;
    
    for (int i = 0; i < 5000; ++i) {
        spinlock_lock(lock);
        shared_value++;
        spinlock_unlock(lock);
    }
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    spinlock_t lock;

    // 初始化自旋锁
    spinlock_init(&lock);

    // 输出初始共享变量值
    printf("Before: shared_value = %d\n", shared_value);

    // 创建两个线程
    pthread_create(&thread1, NULL, thread_function, &lock);
    pthread_create(&thread2, NULL, thread_function, &lock);

    // 等待两个线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // 输出最终共享变量值
    printf("After: shared_value = %d\n", shared_value);

    return 0;
}

结果如图所示：

无论运行多少次，输出几乎总是一样：

makefile
复制代码
Before: shared_value = 0
After: shared_value = 10000

尝试一下如果去掉如果去掉自旋锁



非常好 👍 我来帮你完整、系统地分析一下这个 **自旋锁版本程序的运行结果与执行过程**。
我们会从线程行为、锁机制、性能特征、以及实验现象四个角度来讲，形成一份标准化的实验分析。

---

## 🧩 一、程序执行流程概述

运行过程大体分为以下几个阶段：

1. **初始化阶段**

   * 主线程调用 `spinlock_init(&lock)`，将 `lock.flag` 置为 `0`，表示锁处于未占用状态。
   * 初始化共享变量 `shared_value = 0`。
   * 输出初始值：

     ```
     Before: shared_value = 0
     ```

2. **线程创建阶段**

   * 主线程使用 `pthread_create()` 创建两个线程：`thread1` 和 `thread2`。
   * 两个线程开始并发执行 `thread_function()`，共用同一个自旋锁 `lock`。

3. **线程执行阶段**

   * 每个线程内部执行循环 `for (int i = 0; i < 5000; ++i)`：

     * 调用 `spinlock_lock(&lock)` 尝试加锁；
     * 获得锁后执行 `shared_value++`；
     * 执行完毕后调用 `spinlock_unlock(&lock)` 释放锁；
     * 再进入下一轮循环。

   由于自旋锁是通过原子操作实现的：

   ```c
   while (__sync_lock_test_and_set(&lock->flag, 1)) {
       // busy-wait
   }
   ```

   只有一个线程能成功把 `flag` 从 `0 → 1`，另一线程若看到 `flag == 1`，则在循环中忙等待直到其变为 0。

4. **线程结束阶段**

   * 每个线程执行完 5000 次加 1 后返回。
   * 主线程使用 `pthread_join()` 等待两个线程都结束。

5. **结果输出阶段**

   * 最终输出：

     ```
     After: shared_value = 10000
     ```

---

## 🔒 二、自旋锁运行机制分析

| 步骤                       | 状态变化            | 说明              |
| ------------------------ | --------------- | --------------- |
| 线程 A 尝试加锁                | flag = 0 → 1    | 成功进入临界区         |
| 线程 B 同时加锁                | flag = 1（锁已被占用） | B 进入自旋等待状态      |
| 线程 A 执行 `shared_value++` | 受保护，不被打断        | 保证原子性           |
| 线程 A 释放锁                 | flag = 1 → 0    | B 可继续执行         |
| 线程 B 获得锁                 | flag = 0 → 1    | B 进入临界区         |
| 重复此过程 5000 次 × 2 线程      | ——              | 最终共享变量稳定为 10000 |

---

## 📈 三、实验运行结果与分析

多次运行的输出示例（结果一致）：

```
Before: shared_value = 0
After: shared_value = 10000
```

### 1. **结果正确性**

* 结果稳定且始终等于 `5000 × 2 = 10000`。
* 表明自旋锁有效地保证了共享变量访问的**互斥性**（mutual exclusion）。

### 2. **线程调度特征**

* 两个线程不断争夺锁的控制权。
* 在某一时刻，只有一个线程能修改 `shared_value`。
* 自旋期间线程不会睡眠，而是**忙等待**（busy-waiting）。

### 3. **性能特点**

* 自旋锁的优势在于：

  * 锁持有时间短时，忙等待比上下文切换更高效。
  * 不涉及系统调用，不会进入内核态。
* 缺点在于：

  * 若锁竞争严重或锁持有时间较长，会浪费 CPU 时间在循环等待中。
  * 在单核 CPU 上特别低效，因为等待线程无法真正并行。

### 4. **线程交替行为（逻辑示意）**

| 时间片 | 线程       | 操作              | 共享变量值 |
| --- | -------- | --------------- | ----- |
| t1  | Thread A | 获取锁 → 加 1 → 释放锁 | 1     |
| t2  | Thread B | 获取锁 → 加 1 → 释放锁 | 2     |
| t3  | Thread A | 加 1             | 3     |
| …   | …        | …               | …     |
| 最后  | Thread B | 加到 10000        | 10000 |

---

## 🧠 四、实验结论总结

1. **功能正确性：**
   使用自旋锁后，两个线程对共享变量的访问变为互斥，完全消除了数据竞争。

2. **执行结果：**
   程序每次运行结果一致：

   ```
   After: shared_value = 10000
   ```

3. **机制优势：**

   * 锁操作开销小；
   * 不涉及线程睡眠/唤醒；
   * 适合短期、低延迟的临界区。

4. **机制劣势：**

   * 会占用 CPU 忙等待；
   * 若锁持有时间较长，会显著降低并发效率；
   * 在单核系统中，等待线程无法获得执行机会。

---

## 🧩 五、延伸思考

| 锁类型                 | 等待方式 | 优点       | 缺点       | 典型场景     |
| ------------------- | ---- | -------- | -------- | -------- |
| **自旋锁 (spinlock)**  | 忙等待  | 快速响应、低延迟 | 占 CPU    | 短临界区、高并发 |
| **互斥锁 (mutex)**     | 阻塞等待 | 不浪费 CPU  | 上下文切换开销大 | 临界区耗时较长  |
| **信号量 (semaphore)** | 计数控制 | 可控制资源数量  | 实现复杂     | 资源池管理    |

---

是否希望我帮你把这份分析整理成实验报告格式（包括**实验目的、原理、结果、结论**四个标准部分）？那样可以直接放进实验文档里用。








