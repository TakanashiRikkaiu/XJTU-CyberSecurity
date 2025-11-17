/* 管道通信实验程序——无锁版本 */
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int pid1, pid2;

int main() {
    int fd[2];
    char InPipe[5000]; // 缓冲区 ≥ 4001 即可
    char c1 = '1', c2 = '2';

    pipe(fd); // 创建管道

    while ((pid1 = fork()) == -1);  // 创建子进程1失败则等待
    if (pid1 == 0) {  // 子进程1
        for (int i = 0; i < 2000; i++)
            write(fd[1], &c1, 1);    // ★ 写2000个'1'
        exit(0);
    }
    else {
        while ((pid2 = fork()) == -1); // 创建子进程2失败则等待

        if (pid2 == 0) {  // 子进程2
            for (int i = 0; i < 2000; i++)
                write(fd[1], &c2, 1);  // ★ 写2000个'2'
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
