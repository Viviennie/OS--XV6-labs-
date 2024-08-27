#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 递归函数，用于处理当前进程的素数筛选，并将剩余数据传递到下一个进程
void prime_filter(int input_pipe[2]) {
    int prime;
    int n;

    // 关闭输入管道的写端，因为当前进程只需要从该管道读取数据
    close(input_pipe[1]);

    // 从管道读取第一个数字，这个数字就是当前进程要处理的素数
    if (read(input_pipe[0], &prime, sizeof(prime)) != sizeof(prime)) {
        fprintf(2, "Error in reading prime from pipe.\n");
        exit(1);
    }

    // 输出当前的素数
    printf("prime %d\n", prime);

    // 尝试从输入管道读取下一个数字
    if (read(input_pipe[0], &n, sizeof(n)) == sizeof(n)) {
        int new_pipe[2];
        pipe(new_pipe);

        // 父进程继续处理数据，并将剩余数据传递到下一个进程
        if (fork() != 0) {
            close(new_pipe[0]);  // 关闭新的管道的读取端，父进程只负责写入
            if (n % prime != 0) {
                write(new_pipe[1], &n, sizeof(n));  // 如果n不能被当前素数整除，写入新管道
            }

            // 继续读取并筛选管道中的剩余数字
            while (read(input_pipe[0], &n, sizeof(n)) == sizeof(n)) {
                if (n % prime != 0) {
                    write(new_pipe[1], &n, sizeof(n));
                }
            }

            close(input_pipe[0]);  // 关闭当前管道的读取端
            close(new_pipe[1]);    // 关闭新的管道的写入端
            wait(0);               // 等待子进程结束
        }
        // 子进程继续递归处理下一层的素数筛选
        else {
            prime_filter(new_pipe);
        }
    }

    // 关闭管道，结束当前进程
    close(input_pipe[0]);
    exit(0);
}

int main(int argc, char* argv[]) {
    int initial_pipe[2];
    pipe(initial_pipe);

    // 创建第一个子进程，开始处理素数筛选
    if (fork() == 0) {
        prime_filter(initial_pipe);
    } 
    // 父进程负责向管道中写入初始的数字序列
    else {
        close(initial_pipe[0]);  // 父进程不需要读取管道，关闭读取端
        for (int i = 2; i <= 35; ++i) {
            // 写入数字到管道，每个数字占用4个字节
            if (write(initial_pipe[1], &i, sizeof(i)) != sizeof(i)) {
                fprintf(2, "Failed to write %d into the pipe.\n", i);
                exit(1);
            }
        }
        close(initial_pipe[1]);  // 关闭写入端，表示数据写入完成
        wait(0);  // 等待子进程结束
        exit(0);  // 父进程结束
    }
    exit(0);
}


