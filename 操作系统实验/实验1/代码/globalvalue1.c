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
    else {  // 父进程
        g -= 5;  
        pid1 = getpid();
        printf("parent: pid1 = %d, g(after -5) = %d\n", pid1, g);
        wait(NULL);
    }


    g *= 2;  
    printf("pid %d: g(after *2) = %d\n", getpid(), g);

    return 0;
}
