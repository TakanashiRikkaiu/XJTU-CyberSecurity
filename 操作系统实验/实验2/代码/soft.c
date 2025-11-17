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
    signal(SIGALRM, inter_handler);   // <-- 超时报警信号

    pid_t pid1=-1, pid2=-1;

    while (pid1 == -1) pid1 = fork();
    if (pid1 > 0) {

        while (pid2 == -1) pid2 = fork();
        if (pid2 > 0) {

            // ===== 父进程 =====

            printf("Parent running... Press Ctrl+C or Ctrl+\\ within 5 seconds.\n");

            alarm(5);   // <-- 设置超时 5 秒自动发 SIGALRM 给父进程

            // 等待信号（SIGINT / SIGQUIT / SIGALRM）
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
