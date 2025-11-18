# 实验2  
## 2.1软中断  
实验前准备，使用man命令查看各系统调用帮助手册  
问题：华为云貌似没有这个包，一开始一直显示查找不到这些指令，上网搜索在openeuler社区得到答案，安装man-pages包后解决。  
fork  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/a4da7078-7686-46f0-b743-0ca029a401c8" />  
用于创建子进程，复制父进程状态，常用于多进程编程
kill  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/717c32d9-5af0-48c3-b777-f3a7874c6b37" />  
int kill(pid_t pid, int sig);   pid：目标进程 ID ，sig：信号号。  
成功：0，失败：-1    向进程发送信号，用于终止进程  
exit  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/088bd6a8-ebf1-47dc-adbd-0287c7addddf" />  
终止进程，清理资源  
signal  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/be9fbb4b-7c07-4c7c-9aa3-212e273662fd" />  
signum：信号号 ，handler：处理函数指针  
安装/修改信号处理器，处理中断信号
sleep  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/0e5b6588-5ac8-4382-843c-0d1afbc6b0b7" />  
暂停当前进程指定秒数，常用于延迟执行或轮询  

根据流程图编写软中断通信程序：  
    - 使用`fork()`创建两个子进程。  
        - 使用`signal()`让父进程捕捉键盘中断信号（5s内按下Delete键或Quit键）。  
        - 父进程接收到SIGINT或SIGQUIT信号后，使用`kill()`向两个子进程分别发送信号16和17。  
        - 子进程接收对应信号后，输出信息并终止。

补全程序运行： 
```
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0;        // 父进程收到中断信号
int child_exit = 0;  // 子进程退出标记

// 父进程处理 SIGINT / SIGQUIT / SIGALRM
void inter_handler(int sig) {
    flag = 1;
}

// 子进程收到父进程信号时退出
void child_handler(int sig) {
    child_exit = 1;
}

void waiting() {
    while (!child_exit)
        pause();
}

int main() {

    // 父进程捕捉三类信号：SIGINT, SIGQUIT, SIGALRM
    signal(SIGINT, inter_handler);
    signal(SIGQUIT, inter_handler);
    signal(SIGALRM, inter_handler);   // 超时报警信号

    pid_t pid1=-1, pid2=-1;

    while (pid1 == -1) pid1 = fork();
    if (pid1 > 0) {

        while (pid2 == -1) pid2 = fork();
        if (pid2 > 0) {
            
            printf("Parent running... Press Ctrl+C or Ctrl+\ within 5 seconds.\n");
            // ===== 父进程 =====
            alarm(5);   // 超时5秒自动发SIGALRM给父进程

            // 等待信号
            while (!flag)
                pause();

            // 给两个子进程发信号
            kill(pid1, 16);
            kill(pid2, 17);

            wait(NULL);
            wait(NULL);

            printf("\nParent process is killed!!\n");

        } else {

            // ===== 子进程2 =====
            signal(17, child_handler);
            waiting();
            printf("\nChild process2 is killed by parent!!\n");
            return 0;
        }

    } else {

        // ===== 子进程1 =====
        signal(16, child_handler);
        waiting();
        printf("\nChild process1 is killed by parent!!\n");
        return 0;
    }

    return 0;
}

```
等待5s后    
<img width="840" height="675" alt="image" src="https://github.com/user-attachments/assets/89c33b32-ebae-40b3-bac2-fd1995bd33a3" />  
按下ctrl+c  
<img width="859" height="680" alt="image" src="https://github.com/user-attachments/assets/3a60410f-367b-43a3-ad4d-0d0982318b6d" />  
ctrl+\
<img width="841" height="680" alt="image" src="https://github.com/user-attachments/assets/997ba255-a734-4668-9e93-68fad8452e86" />  
输出顺序不固定  

预期三种中断输出一致：子进程1先打印消息，子进程2次之，父进程最后         
与猜测完全一致，多次运行输出稳定，子1通常先出，‘有时2先  

若等待 5s 超时→父进程收到 SIGALRM，随后给两个子进程发信号  
若按下 Ctrl+C 或 Ctrl+\ → 父进程收到对应信号，同样发信号给子进程  

两个子进程打印顺序不固定，因为它们并发执行、响应信号速度不同  

针对信号通信的统一化优化，核心是将父进程向子进程发送的自定义信号16和17统一改为标准信号SIGALRM，14  
功能逻辑保持一致：父进程收到中断后通知子进程退出  

改为闹钟中断：  
使用 SIGALRM 替换自定义信号16与17  
```
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0;        // 父进程收到中断信号
int child_exit = 0;  // 子进程退出标记

// 父进程处理 SIGINT / SIGQUIT / SIGALRM
void inter_handler(int sig) {
    flag = 1;
}

// 子进程收到父进程信号时退出
void child_handler(int sig) {
    child_exit = 1;
}

void waiting() {
    while (!child_exit)
        pause();
}

int main() {

    // 父进程捕捉三类信号：SIGINT, SIGQUIT, SIGALRM
    signal(SIGINT, inter_handler);
    signal(SIGQUIT, inter_handler);
    signal(SIGALRM, inter_handler);

    pid_t pid1=-1, pid2=-1;

    while (pid1 == -1) pid1 = fork();
    if (pid1 > 0) {

        while (pid2 == -1) pid2 = fork();
        if (pid2 > 0) {

            // ===== 父进程 =====

            printf("Parent running... Press Ctrl+C or Ctrl+\\ within 5 seconds.\n");

            alarm(5);   // 5秒超时自动产生 SIGALRM

            // 等待任意一个信号
            while (!flag)
                pause();

            // 父进程不再向子进程发 16、17 信号
            kill(pid1, SIGALRM);   // 改为 SIGALRM
            kill(pid2, SIGALRM);   // 改为 SIGALRM

            wait(NULL);
            wait(NULL);

            printf("\nParent process is killed!!\n");

        } else {

            // ===== 子进程2 =====

        
            // 子进程捕获14号信号

            signal(SIGALRM, child_handler);   // 改为 SIGALRM

            waiting();
            printf("\nChild process2 is killed by parent!!\n");
            return 0;
        }

    } else {

        // ===== 子进程1 =====

        // 子进程捕获14号信号

        signal(SIGALRM, child_handler);   // 改为 SIGALRM


        waiting();
        printf("\nChild process1 is killed by parent!!\n");
        return 0;
    }

    return 0;
}

```
同样等待5s  
<img width="838" height="450" alt="image" src="https://github.com/user-attachments/assets/f58e3756-816e-441b-820c-f15c10a8f5d6" />  
或者是按下按键  
<img width="840" height="677" alt="image" src="https://github.com/user-attachments/assets/4eb4d4d0-5e25-4e0f-8d58-49cbc26b2dcd" />  
没有明显区别，结果基本一致，运行稳定  

问题：发现图中子进程1和2输出的内容之间缺少空行，是因为进程并发执行，终端渲染时空行被忽略  

kill用了2次，进程间通信，实现父进程通知子进程退出 子进程响应信号输出并终止。  
 外部kill，主动exit：外部kill强制终止，可能遗漏资源清理；主动exit释放资源、通知父进程。  
 主动exit更好 


## 2.2管道通信  
有锁情况 
```c
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int pid1, pid2;

int main() {
    int fd[2];
    char InPipe[4005]; // 读缓冲区
    char c1 = '1', c2 = '2';

    pipe(fd); // 创建管道

    while ((pid1 = fork()) == -1);  // 创建子进程1失败则一直等

    if (pid1 == 0) {  // 子进程 1
        lockf(fd[1], 1, 0);          //  锁定管道写端
        for (int i = 0; i < 2000; i++)
            write(fd[1], &c1, 1);    //  连续写2000个 '1'
        sleep(5);                    // 让父进程有时间读
        lockf(fd[1], 0, 0);          //  解锁管道写端
        exit(0);
    }
    else {
        while ((pid2 = fork()) == -1);  // 创建子进程2失败则一直等

        if (pid2 == 0) {  // 子进程 2
            lockf(fd[1], 1, 0);
            for (int i = 0; i < 2000; i++)
                write(fd[1], &c2, 1);   //  连续写2000个 '2'
            sleep(5);
            lockf(fd[1], 0, 0);
            exit(0);
        }
        else {
            wait(0);                    //  等待子进程1结束
            wait(0);                    // 等待子进程2结束
            read(fd[0], InPipe, 4000);  //  一次读出4000字符
            InPipe[4000] = '\0';        // 字符串结束符
            printf("%s\n", InPipe);
            exit(0);
        }
    }
}

```
<img width="1734" height="1095" alt="image" src="https://github.com/user-attachments/assets/31eb7464-c19b-4ed7-ab69-91d6079434ec" />    
互斥锁， 同一时刻只有一个进程能写管道，先写 2000 个 1，再写 2000 个 2，也有可能反过来  

去掉锁  
```c
/* 管道通信实验程序——无锁版本 */
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int pid1, pid2;

int main() {
    int fd[2];
    char InPipe[5000]; // 缓冲区
    char c1 = '1', c2 = '2';

    pipe(fd); // 创建管道

    while ((pid1 = fork()) == -1);  // 创建子进程1失败则等待
    if (pid1 == 0) {  // 子进程1
        for (int i = 0; i < 2000; i++)
            write(fd[1], &c1, 1);    //  写2000个'1'
        exit(0);
    }
    else {
        while ((pid2 = fork()) == -1); // 创建子进程2失败则等待

        if (pid2 == 0) {  // 子进程2
            for (int i = 0; i < 2000; i++)
                write(fd[1], &c2, 1);  //  写2000个'2'
            exit(0);
        }
        else {
            wait(0);      // 等待子进程1
            wait(0);      // 等待子进程2

            read(fd[0], InPipe, 4000); // 读 4000 字节
            InPipe[4000] = '\0';       // 字符串结束符

            printf("%s\n", InPipe);
            exit(0);
        }
    }
}
```
<img width="1734" height="1095" alt="image" src="https://github.com/user-attachments/assets/e803ae5c-fca0-4917-9d91-65c7d8d32308" />  
两个进程同时写同一个管道，1和2交替混杂，没有规律  

这是因为write不是原子操作  

同步  
read(fd[0]) 会阻塞，直到写端写入数据  
 父进程可以等待子进程写完  

互斥  
多个写进程同时写时不会互斥  
必须使用 lockf 来人为加锁  

问题：开始时漏写#include <sys/wait.h> ，导致warning，但之前实验可以运行，这次不懂为什么不行，加上后解决。  

预期和实际情况相符。  
无互斥的后果：1 和 2 写入随机交叉；  
无同步的后果：父进程可能在子进程写入之前读取，输出内容长度不对  

## 2.3 内存分配  
补全代码，使用3种算法，碎片分析处理  

使用链表模拟空闲块  
实现 FF / BF / WF 三种算法  
对比其分配行为  
分析内/外碎片  
实现空闲块合并与紧缩  

菜单：  
1 - Set memory size (default=1024)  
2 - Select memory allocation algorithm  
3 - New process  
4 - Terminate a process  
5 - Display memory usage  
0 - Exit  

初始内存情况如下：  
<img width="738" height="516" alt="image" src="https://github.com/user-attachments/assets/7283e51b-4318-4c0e-b3ac-41a8a18ad0fd" />  

①使用FF算法，连续分配大小为20，35，400，查看资源分配情况  
<img width="738" height="316" alt="image" src="https://github.com/user-attachments/assets/6dc8bb80-758a-4584-a6c8-f947a94ab6e4" />  
<img width="738" height="64" alt="image" src="https://github.com/user-attachments/assets/5ece683d-5303-429d-ad12-336bf2135cd3" />  

释放进程2，观察  
<img width="738" height="360" alt="image" src="https://github.com/user-attachments/assets/f77bde1d-6776-48e4-8e5f-3b8a2850381a" />  

发现空闲区被从前往后分割  
这时分配一个大小为25的  
<img width="738" height="340" alt="image" src="https://github.com/user-attachments/assets/178ca9ef-bf0c-4e85-9a42-b36a464c5124" />  
发现放在了刚才释放的区域  

②使用BF，分配100，200，300，如图  
<img width="738" height="315" alt="image" src="https://github.com/user-attachments/assets/48251496-11ef-451c-acfe-da7c43912a42" />  

释放2，放入120<200：  
<img width="738" height="830" alt="image" src="https://github.com/user-attachments/assets/09699e89-0d04-49b8-a1fb-5ea2d04cfc6f" />  

120放到那个最小空闲块里面，而不是后面的更大块。  

③使用WF，分配100，50，300  
<img width="738" height="310" alt="image" src="https://github.com/user-attachments/assets/2172d6ab-6798-4dcb-9cbe-3264a04e6f77" />  

释放1，2  
<img width="738" height="314" alt="image" src="https://github.com/user-attachments/assets/84043449-4289-4a17-ad61-5c3376a23b10" />  


分配大小为80的  
<img width="738" height="817" alt="image" src="https://github.com/user-attachments/assets/77c51df1-2f67-4299-83b3-5cae45b18998" />  
可见WF会选择最大的空闲块，不是100，而是后面的最大空间  

④分配100，剩余924，再分配920，发现剩余太少，被合并成一块，程序会把这4当作内碎片加入920中  
<img width="738" height="276" alt="image" src="https://github.com/user-attachments/assets/36ea09e0-ecbe-450f-876b-6b38bdfd7095" />  

释放1  
<img width="738" height="288" alt="image" src="https://github.com/user-attachments/assets/2fabca95-4f7e-4d04-9f23-a45be99560be" />    

可见100-1024空间为连续大块   

⑤分配200，300，200，情况如下  
<img width="738" height="318" alt="image" src="https://github.com/user-attachments/assets/29b4cbf9-551a-4102-8c73-66297d6795b8" />  

杀死进程2，大小300  
<img width="738" height="318" alt="image" src="https://github.com/user-attachments/assets/c23518b5-b6fd-4301-a869-cfc253773fc4" />  

分配大小500，没有任何一块满足，合并所有空闲碎片形成一个大块，再分配给进程  
<img width="738" height="330" alt="image" src="https://github.com/user-attachments/assets/7d131e4c-7d3e-4894-927e-0cc3bf0ee26d" />  

⑥分配100，200，150：  
<img width="738" height="311" alt="image" src="https://github.com/user-attachments/assets/81cd32eb-144d-4415-b51d-10611878ad8d" />  

杀死进程1，2，两块相邻空闲合并为一块  
<img width="738" height="294" alt="image" src="https://github.com/user-attachments/assets/db1b546d-32ba-43d6-b851-4a19b5308b0d" />  

FF：地址递增链表，从首顺序查找，分割剩余。  
BF：大小升序，选最小满足。  
WF：大小降序，选最大满足。  
算法的思想和特点，提高性能：     

首次适应算法  
思想：从链表头部开始顺序扫描，找到第一个大小满足进程请求的空闲分区，即从中分割所需大小的块分配给进程，剩余部分作为新空闲块保留在链表中。  
优点：实现简单，分配速度快。  
缺点：易产生外部碎片，低地址区碎片累积，高地址大块易被分割  
适用：内存利用率中等，碎片管理需结合紧缩（。  

最佳适应算法  
思想：空闲分区链表按大小由小到大排序。扫描链表，找到第一个满足进程请求大小的分区，从中分割分配，剩余作为新空闲块插入链表。  
优点：最小化剩余碎片（选择最紧凑块，减少外碎片），内存利用率较高，尤其适合多小进程场景。  
缺点：需分配/回收时间开销大，易产生许多小碎片  
适用：碎片敏感环境，但性能开销需优化  

[程序](代码/allo.c)

最差适应算法  
思想：空闲分区链表按大小由大到小排序。扫描链表，找到第一个满足进程请求大小的分区，从中分割分配，剩余作为新空闲块插入链表。  
优点：保留较大剩余块，便于后续大进程分配，减少小碎片产生  
缺点：分配时间开销大，易产生内碎片   
适用：进程大小变异大、需支持大块分配的系统。  

内外碎片：  
内碎片：发生在已分配的内存块内部。进程请求大小与实际分配块大小不匹配，导致块内部分空间闲置但无法分配给其他进程。  
外碎片：发生在空闲内存块之间。空闲空间总量足够，但分散成多个小块，无法形成连续大块满足新进程请求，合并相邻，紧缩   

合并空闲块方法： 当释放的内存区域与相邻空闲块相连时，将它们合并成一个连续大块，提高内存利用率。 

问题：紧缩时地址重叠。  
compact_memory() 把所有已分配块紧凑到低地址，并生成一个单一空闲块。
