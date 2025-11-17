/* 管道通信实验程序（补全版）*/
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
        lockf(fd[1], 1, 0);          // ★ 锁定管道写端
        for (int i = 0; i < 2000; i++)
            write(fd[1], &c1, 1);    // ★ 连续写2000个 '1'
        sleep(5);                    // 让父进程有时间读
        lockf(fd[1], 0, 0);          // ★ 解锁管道写端
        exit(0);
    }
    else {
        while ((pid2 = fork()) == -1);  // 创建子进程2失败则一直等

        if (pid2 == 0) {  // 子进程 2
            lockf(fd[1], 1, 0);
            for (int i = 0; i < 2000; i++)
                write(fd[1], &c2, 1);   // ★ 连续写2000个 '2'
            sleep(5);
            lockf(fd[1], 0, 0);
            exit(0);
        }
        else {
            wait(0);                    // ★ 等待子进程1结束
            wait(0);                    // 等待子进程2结束
            read(fd[0], InPipe, 4000);  // ★ 一次读出4000字符
            InPipe[4000] = '\0';        // ★ 字符串结束符
            printf("%s\n", InPipe);
            exit(0);
        }
    }
}
