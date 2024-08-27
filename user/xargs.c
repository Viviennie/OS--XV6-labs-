#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // 检查命令行参数是否足够
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }

    char buf[512];
    char *xargv[MAXARG];
    int i, n;
    int buf_index = 0;

    // 初始化xargv，先填充argv里的参数
    for (i = 0; i < argc - 1; i++) {
        xargv[i] = argv[i + 1];
    }

    while ((n = read(0, buf + buf_index, sizeof(buf) - buf_index - 1)) > 0) {
        buf_index += n;
        buf[buf_index] = '\0';  // 确保字符串终止

        char *p = buf;
        char *line_start = buf;

        while ((p = strchr(line_start, '\n')) != 0) {
            *p = '\0';  // 替换换行符为字符串终止符
            xargv[argc - 1] = line_start;

            if (fork() == 0) {
                exec(xargv[0], xargv);
                fprintf(2, "exec failed\n");
                exit(1);
            } else {
                wait(0);
            }

            line_start = p + 1;
        }

        // 如果存在残留未处理的数据，将其移到缓冲区的开头
        if (line_start < buf + buf_index) {
            buf_index = strlen(line_start);
            memmove(buf, line_start, buf_index);
        } else {
            buf_index = 0;
        }
    }

    // 如果有未处理的行
    if (buf_index > 0) {
        buf[buf_index] = '\0';
        xargv[argc - 1] = buf;

        if (fork() == 0) {
            exec(xargv[0], xargv);
            fprintf(2, "exec failed\n");
            exit(1);
        } else {
            wait(0);
        }
    }

    exit(0);
}

