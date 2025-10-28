我是傻逼     makefile？是什么
# 实验一     
## 1.1进程实验
### 1.1.1wait函数
搭建好华为云服务器后开始实验，直接提取图片代码运行。由于所给程序使用了wait函数且缺少头文件，在使用命令行编译时会报错，如图所示：
<div align="center">
  <img width="1685" height="174" alt="image" src="https://github.com/user-attachments/assets/1bcf9792-5d23-467b-ac2e-23d05e967d4d" />
  <br>缺少头文件的运行结果
</div>

不过报错并不影响程序运行。对于所给程序，刚开始时我为了观察输入方便，在每个输出内加入了换行符\n,所输出的结果如下图所示：
<div align="center">
  <img width="571" height="704" alt="image" src="https://github.com/user-attachments/assets/4727eafb-6309-4f92-b41c-38862ffada5b" />
</div>
<div align="center">
  <img width="524" height="704" alt="image" src="https://github.com/user-attachments/assets/1e31b4a8-2c92-4235-9636-cb6893d52cae" />  
  <br>存在换行符的情况
</div>

如图所示，每次的输出分为4行，易于观察，但是奇怪的是输出的顺序。由于wait函数的影响，我认为应该是先输出子进程的两个pid，而后才是父进程，就像这样：  
child: pid = 0  
child: pid1 = 4890  
parent: pid = 4890  
parent: pid1 = 4889    
然而我在安装了基于OpenEuler的Deepin桌面系统的虚拟机上对相同的程序进行编译运行，结果却是这样的：
<div align="center">
 <img width="526" height="684" alt="image" src="https://github.com/user-attachments/assets/cee314d3-27cc-4b1e-9c15-e1f550cf22ca" />
</div>

<div align="center">
 <img width="536" height="676" alt="image" src="https://github.com/user-attachments/assets/b8e26388-a37d-462a-99bc-d1a70910f7ec" />
  <br>在虚拟机上运行
 </div> 


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
查询资料得知，输出顺序取决于操作系统调度，具体原因：fork() 创建子进程后，父进程和子进程是并发执行的。操作系统内核的调度器决定哪个进程先运行，这取决于 CPU 负载、调度算法和随机因素（如时钟中断）。    
实际上，代码中每个printf都有\n，所以理论上每行独立刷新，但如果缓冲区共享或切换时机巧合，仍可能部分重叠，这应该是主要原因。    
删除各行的\n后,代码如下：  

```
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
        printf("child: pid = %d", pid);   /* A */
        printf("child: pid1 = %d", pid1); /* B */
    }
    else { /* parent process */
        pid1 = getpid();
        printf("parent: pid = %d", pid);   /* C */
        printf("parent: pid1 = %d", pid1); /* D */
        wait(NULL);
    }

    return 0;
}
```

再次使用华为云服务器进行编译运行，得到了正确的输出顺序：
<div align="center">
  <img width="1455" height="317" alt="image" src="https://github.com/user-attachments/assets/7ae545c1-58a6-4562-bf4d-ccef3ea02a46" />
  <br>删除\n后的运行结果
</div>

初步想法：运行几十次获得统计性的一般规律，限于篇幅仅运行10次，可以得到结果规律如下：    
多次运行程序时，父子进程的PID似乎在连续递增，前后差值基本为2，偶尔出现3；  
每一次运行结果中，子进程的pid总是为0，且子进程的pid1与父进程的pid始终相等，父进程的pid始终比其pid1大1。  
原因分析：  
在子进程中，fork()返回值pid始终为0。
子进程的 pid1 = getpid() = 子进程的实际 PID。
父进程的 pid = fork() 返回值 = 子进程的 PID。
因此，二者相等，这是因为父进程通过这个值来“引用”子进程。
关于父进程中pid与pid1插值为1的关系，查询得到的解释是父PID通常比子PID小，但不一定是正好相差1。
至于前后两次间的pid关系，我的理解是：每次运行程序会创建2个新进程，所以 PID 总是递增两个；但是若中间有系统进程插入，可能会略有跳变，比如这个3，但整体仍是递增的。    
[参考资料：进程标识与进程创建（pid, fork）](https://blog.csdn.net/daaikuaichuan/article/details/82779011)      
去除wait后运行：
<div align="center">
<img width="1476" height="282" alt="image" src="https://github.com/user-attachments/assets/9c1ab1ef-8303-4083-8cb5-979be170ef3e" />
</br>去除wait函数的结果
</div>
pid内容关系类似上面的，但是输出顺序发生变化。对比以上两种情况发现，wait存在时，原本的程序输出都是子进程先于父进程，但在去掉wait函数后，父、子进程的输出先后次序就不一定了。    
分析：有wait()时，父进程会执行到wait(NULL)，这条语句会让父进程暂停执行，直到子进程结束。所以，子进程几乎总是先完成输出，父进程最后打印、最后退出，输出顺序比较稳定。    
     去掉wait()后，父进程不再等待子进程，它们并发执行，结果就会有前面说的3种可能。同时父进程可能先退出，导致出现孤儿进程。    
而一开始wait存在时却交替输出就是因为printf缓冲区有换行符才会刷新缓冲区，将缓冲区中的内容显示出来。
按照此逻辑，如果使用了wait且包含了头文件，应该是成功调用函数，父等子结束。  
尝试了一下把wait放到父进程输出语句之前，运行代码：  
<div align="center">
pid2
</div>  
这次输出顺序均为子先，父后。  
分析：查阅资料，wait()的作用是让父进程等待子进程结束并回收资源；<sys/wait.h>的作用是声明wait()函数，让编译器能正确检查参数与返回类型。不写wait()只是让系统自己回收资源，但会导致输出乱序和孤儿进程。

### 1.1.2全局变量
为方便观察，保留换行符，并补全头文件后，添加一个全局变量g，代码如下；  

```
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
```

子进程自加，父进程自减。运行多次g输出结果不变：
<div align="center">
<img width="585" height="176" alt="image" src="https://github.com/user-attachments/assets/7c5d1165-f78a-4962-a23f-b0ccbba0ce45" />
</div>

为了探究在父子进程中同一变量的关系，我查看了一篇文章  
[父子进程的内存变量关系](https://blog.csdn.net/qq_26836575/article/details/82014685)：

> fork() 创建子进程时继承了父进程的数据段、代码段、栈段、堆，  
> 注意从父进程继承来的是虚拟地址空间，同时也复制了页表（没有复制物理块）。  
> 因此，此时父子进程拥有相同的虚拟地址，映射的物理内存也是一致的（独立的虚拟地址空间，共享父进程的物理内存）。

一篇文章说：  
父子进程打印出来的全局变量地址一样，原因在于这个地址是虚拟地址，  
而因为子进程的创建是复制了父进程的虚拟地址空间的，  
所以这两个变量的虚拟地址也是一样的。  
（我们打印变量的地址都是虚拟地址，物理内存地址是不能够直接访问的）

**二者物理地址不同。**

**分析：**  
父子进程各自维护一份独立的变量，fork()发生时，变量 g 的值被复制到子进程中。  
因此，父进程 g = 5，子进程 g = 15，互不影响。
  
return前增加操作，修改代码如下：  

```
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
```  

多次运行可得相同结果。  
<div align="center">
<img width="880" height="201" alt="image" src="https://github.com/user-attachments/assets/2bb70819-7433-43b9-929d-4b2aaca9595b" />
</div>      
分析：和上面类似，每个进程独立执行g *= 2，父子进程的结果互不影响。子进程：(10+5)*2=30，父进程：(10-5)*2=10。

### 1.1.3调用外部程序

编写被调用程序代码：  

```
#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("system_call PID: %d, 我的学号:2233514228\n", getpid());
    return 0;
}
```

<div align="center">
  <img width="878" height="90" alt="image" src="https://github.com/user-attachments/assets/12274c08-0bdf-4bc1-b524-22297ba0b680" />
</div>  
不支持中文输出：我的学号 这几个字，出现乱码。
解决：配置中文环境失败，好像是程序编码格式问题，于是改中文为英文：    

sys改  
两个程序和图片  
system：

```
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
```

<div align="center">
 <img width="855" height="200" alt="image" src="https://github.com/user-attachments/assets/93637897-dd5c-4f84-a2e0-694a15cd1eb0" />
</div>  

exec：

```
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
        execl("./system_call", "system_call", NULL);
        perror("execl failed"); 
    }
    else {
        wait(NULL);
        printf("child process finished. parent PID: %d\n", getpid());
    }

    return 0;
}
```

<div align="center">
<img width="665" height="179" alt="image" src="https://github.com/user-attachments/assets/09206440-12c2-41df-875d-4359e2737c30" />
</div>

分析结果：调用system后外部程序中的pid比子进程pid大1，而调用execl后外部程序中的pid与子进程相同。这是为什么：？
根据https://blog.csdn.net/hza419763578/article/details/84305468      
使用 system() 时：子进程再生成一个新进程来运行外部命令，system_call是孙进程；system_call 的 PID 与子进程不同，父 PID 为子进程。  
使用 exec() 时：子进程被替换为新程序；system_call 与子进程共用同一个 PID；ystem_call的父进程为原父进程。  

## 1.2线程实验
开始时，设计程序，创建两个子线程，两线程分别对同一个共享变量多次操作，观察输出结果。
编译报错，报错信息显示pthread 库中的函数是未定义的引用。  
解决方法：由于pthread 库不是Linux 系统默认的库，链接时需要使用库libpthread.a，所以使用pthread_create 创建线程时，要在编译中加-lpthread 参数。  

```
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int count = 0;  // 共享变量

void* thread_inc(void* arg) {  // 线程1：自增
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 6000; i++) {
        count++;  // 加1操作
    }
    printf("Thread %ld finished. Current count = %d\n", tid, count);
    return NULL;
}

void* thread_dec(void* arg) {  // 线程2：自减
    long tid = (long)pthread_self();
    printf("Thread %ld started\n", tid);
    for (int i = 0; i < 6000; i++) {
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
```

选择次数6000，如果二者独立，理论上输出应该是0。
加锁or not？？？尝试
运行十余次，出现了多次现象，如图所示
<div align="center">
<img width="760" height="504" alt="image" src="https://github.com/user-attachments/assets/529ada1d-1b4a-4516-b6ac-6dcb5e52bbf4" />
<img width="797" height="506" alt="image" src="https://github.com/user-attachments/assets/97cdf279-342d-45c5-a901-87ce234c0cc5" />
<img width="777" height="504" alt="image" src="https://github.com/user-attachments/assets/b8c4152b-1e62-4760-8c1e-ba5408894a24" />
<img width="780" height="507" alt="image" src="https://github.com/user-attachments/assets/367d3614-05df-4d28-8cd9-9aa915ec3afc" />
</div>
其实输出还是有不少0的，偶尔出现其他数字。但我感觉在没有同步机制的情况下，两线程对同一个共享变量进行修改时，结果应该是不确定的，而且几乎不可能完全等于理论值0。  

查阅资料得知，现实运行中（尤其是循环较少，比如6000次），往往会出现一种“假象”——运行了多次，结果反而一直是0。  

这其实是因为循环次数太少，6000次在现代 CPU 上只需要几微秒完成。调度切换（线程间切换）往往需要上万微秒级别，线程可能还没被切换就已经跑完了。  

增加操作数☞60000，程序thread0
<div align="center">
<img width="882" height="787" alt="image" src="https://github.com/user-attachments/assets/dcab8922-6c62-4712-a117-2b1b9295d045" />
<img width="797" height="762" alt="image" src="https://github.com/user-attachments/assets/1ef04413-f515-419c-9dee-ca6c4cdeecc5" />
<img width="825" height="757" alt="image" src="https://github.com/user-attachments/assets/4c2a1d75-0231-4588-bfe8-c23b60b8f19b" />
<img width="790" height="256" alt="image" src="https://github.com/user-attachments/assets/4f9ffc9f-e8a2-4f92-afc1-1f5336685f35" />
</div>
4张图，此时次次输出均与理论值不符。说明出现线程共享资源时的竞争现象。  
分析：count++与count--都不是原子操作，会分为读、修改、写三个步骤，当两个线程几乎同时访问时，会出现“读到旧值 → 改错 → 覆盖”的情况，所以最终结果通常不是 0，而是一个随机波动的小整数。    

因此需要信号量或互斥锁实现互斥访问，就能让最终结果稳定为0    
①thread1 增加信号量和pv操作，再次运行10次结果如下  
<div align="center">
<img width="867" height="617" alt="image" src="https://github.com/user-attachments/assets/145e5e03-9a28-4b25-acd4-7fe4bd9107cc" />
<img width="867" height="591" alt="image" src="https://github.com/user-attachments/assets/6c3d761f-7069-41d2-9adf-1809a9437688" />
<img width="867" height="783" alt="image" src="https://github.com/user-attachments/assets/fdf9a9e6-6a39-4051-a58b-b27c002dba45" />
</div>
虽然过程中count变化不定，但最终可得输出稳定为0  
分析：代码创建了两个线程：一个负责递增，另一个负责递减。每个线程执行60000次操作，最终count的值应该为0。  
代码使用信号量防止两个线程同时修改共享变量count，从而避免数据竞争导致的错误结果，如果没有信号量，count 最终可能不是 0。  

②thread2 使用互斥锁。  
<div align="center">
<img width="881" height="627" alt="image" src="https://github.com/user-attachments/assets/1fadaef7-a40f-4719-9222-65dffbe65d24" />
</div>
同上
分析：使用POSIX线程互斥锁来保护共享变量count的访问。  
二者效果相同  

尝试灵活运用信号量和PV操作实现线程间的同步互斥。？？？？？？？？？？？？？？？？？？？不太会，求助博客和ai
综合运用，2张图，thread3  
<div align="center">
<img width="874" height="876" alt="image" src="https://github.com/user-attachments/assets/eeee8ad1-6900-4475-82b6-997a97982429" />
<img width="738" height="844" alt="image" src="https://github.com/user-attachments/assets/3eb482cd-d3b9-4c9f-9532-89815eb794b4" />
</div>
分析pid关系  


2种方法调用
由于之前编写system_call.c时在代码中使用中文。而终端中输出似乎并不支持，于是修改代码，再次运行如下：
sys改png  
运行修改过的程序  
<div align="center">
<img width="847" height="89" alt="image" src="https://github.com/user-attachments/assets/d1113153-e29a-46c9-8b32-935661587fc4" />
</div>  
修改完代码，编写程序，在进程中创建两个线程，分别使用两个函数方法进行调用
①system()
代码：

```
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* thread_call(void* arg) {
    long id = (long)arg;
    printf("thread%ld created\n", id);
    printf("thread%ld tid = %ld ,pid = %d\n", id, pthread_self(), getpid());
    system("./system_call");
    printf("thread%ld systemcall return\n", id);
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
```

结果：  
<div align="center">
<img width="696" height="762" alt="image" src="https://github.com/user-attachments/assets/456ace5b-e57c-4a87-9218-c9fc15065994" />
<img width="697" height="763" alt="image" src="https://github.com/user-attachments/assets/985b3aa9-3b55-4aee-8def-09a8e38029ee" />
</div>  
分析：


②:exec
代码：

```
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

```

结果：  
<div align="center">
<img width="879" height="789" alt="image" src="https://github.com/user-attachments/assets/7b54b8be-6395-42fa-86d5-72b2d4f44d01" />
<img width="735" height="756" alt="image" src="https://github.com/user-attachments/assets/9bd4f1a2-7428-4897-8acc-f849291af28e" />
</div>  
分析：



## 1.3自旋锁实验
这是啥？https://www.cnblogs.com/cxuanBlog/p/11679883.html
补全代码如下：

```
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
```

结果如图所示：  
<div align="center">
<img width="902" height="450" alt="image" src="https://github.com/user-attachments/assets/4ce9658f-45e4-4750-85e0-d35a5dfa6423" />
</div>  

无论运行多少次，输出几乎总是一样：
Before: shared_value = 0
After: shared_value = 10000  

使用pthreads库实现多线程，并通过原子操作内置函数__sync_lock_test_and_set和__sync_lock_release实现自旋锁。  
尝试一下如果去掉如果去掉自旋锁?  
如果去掉 spinlock_lock/unlock，shared_value++ 不是原子操作（读-改-写三个步骤），两个线程可能同时读到相同值，导致最终值 < 10000








 





。








