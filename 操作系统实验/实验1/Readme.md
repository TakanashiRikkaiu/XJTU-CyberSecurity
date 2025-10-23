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















