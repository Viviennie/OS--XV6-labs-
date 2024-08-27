#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    int fd[2];

    if (pipe(fd) == -1) {
        fprintf(2, "Error: pipe(fd) error.\n");
        exit(1);  // 加上 exit(1); 否则程序继续执行。
    }
    int pid = fork();
    // 子进程
    if (pid == 0){
        char buffer[1];
        read(fd[0], buffer, 1);
        close(fd[0]);// 关闭读取端，表示不再使用
        fprintf(0, "%d: received ping\n", getpid());
        exit(0);  // 子进程结束
    }
    // 创建进程失败
    else if (pid < 0) {
    fprintf(2, "Error: fork() error.\n"); // 输出错误信息到标准错误输出（stderr）
    exit(1); // 直接退出程序
    }
    // 父进程
    else {
        close(fd[0]);  // 关闭未使用的读端
        char buffer[1];
        buffer[0] = 'a';
        write(fd[1], buffer, 1);
        close(fd[1]);// 关闭写端，表示不再写入
        wait(0);  // 等待子进程完成，以避免僵尸进程
        fprintf(0, "%d: received pong\n", getpid());
    }
    exit(0);
}