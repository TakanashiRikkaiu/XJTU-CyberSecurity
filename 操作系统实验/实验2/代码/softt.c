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

            // ================================
            // 改动：父进程不再向子进程发 16、17 信号
            //      统一改为发 14 号信号 (SIGALRM)
            // ================================
            kill(pid1, SIGALRM);   // ← 修改：改为 SIGALRM
            kill(pid2, SIGALRM);   // ← 修改：改为 SIGALRM
            // ================================

            wait(NULL);
            wait(NULL);

            printf("\nParent process is killed!!\n");

        } else {

            // ===== 子进程2 =====

            // ================================
            // 改动：子进程捕获 14 号信号 (SIGALRM)
            // ================================
            signal(SIGALRM, child_handler);   // ← 修改：改为 SIGALRM
            // ================================

            waiting();
            printf("\nChild process2 is killed by parent!!\n");
            return 0;
        }

    } else {

        // ===== 子进程1 =====

        // ================================
        // 改动：子进程捕获 14 号信号 (SIGALRM)
        // ================================
        signal(SIGALRM, child_handler);   // ← 修改：改为 SIGALRM
        // ================================

        waiting();
        printf("\nChild process1 is killed by parent!!\n");
        return 0;
    }

    return 0;
}
